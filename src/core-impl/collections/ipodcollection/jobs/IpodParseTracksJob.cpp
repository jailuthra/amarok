/****************************************************************************************
 * Copyright (c) 2012 Matěj Laitl <matej@laitl.cz>                                      *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#include "IpodParseTracksJob.h"

#include "IpodCollection.h"
#include "IpodMeta.h"
#include "IpodPlaylistProvider.h"
#include "core/interfaces/Logger.h"
#include "core/support/Components.h"
#include "core/support/Debug.h"
#include "core-impl/meta/file/File.h"

#include <gpod/itdb.h>


IpodParseTracksJob::IpodParseTracksJob( IpodCollection *collection )
    : Job()
    , m_coll( collection )
    , m_aborted( false )
{
}

void IpodParseTracksJob::abort()
{
    m_aborted = true;
}

void
IpodParseTracksJob::run()
{
    DEBUG_BLOCK
    Itdb_iTunesDB *itdb = m_coll->m_itdb;
    if( !itdb )
        return; // paranoia

    guint32 trackNumber = itdb_tracks_number( itdb );
    QString operationText = i18nc( "operation when iPod is connected", "Reading iPod tracks" );
    Amarok::Components::logger()->newProgressOperation( this, operationText, trackNumber,
                                                        this, SLOT(abort()) );

    Meta::TrackList staleTracks;
    QSet<QString> knownPaths;

    for( GList *tracklist = itdb->tracks; tracklist; tracklist = tracklist->next )
    {
        if( m_aborted )
            break;

        Itdb_Track *ipodTrack = (Itdb_Track *) tracklist->data;
        if( !ipodTrack )
            continue; // paranoia
        // IpodCollection::addTrack() locks and unlocks the m_itdbMutex mutex
        Meta::TrackPtr proxyTrack = m_coll->addTrack( new IpodMeta::Track( ipodTrack ) );
        if( proxyTrack )
        {
            QString canonPath = QFileInfo( proxyTrack->playableUrl().toLocalFile() ).canonicalFilePath();
            if( !proxyTrack->isPlayable() )
                staleTracks.append( proxyTrack );
            else if( !canonPath.isEmpty() )  // nonexistent files return empty canonical path
                knownPaths.insert( canonPath );
        }

        incrementProgress();
    }

    parsePlaylists( staleTracks, knownPaths );
    emit endProgressOperation( this );
}

void
IpodParseTracksJob::parsePlaylists( const Meta::TrackList &staleTracks,
                                    const QSet<QString> &knownPaths )
{
    IpodPlaylistProvider *prov = m_coll->m_playlistProvider;
    if( !prov || m_aborted )
        return;

    if( !staleTracks.isEmpty() )
    {
        prov->m_stalePlaylist = Playlists::PlaylistPtr( new IpodPlaylist( staleTracks,
            i18nc( "iPod playlist name", "Stale tracks" ), m_coll, IpodPlaylist::Stale ) );
        prov->m_playlists << prov->m_stalePlaylist;  // we dont subscribe to this playlist, no need to update database
        emit prov->playlistAdded( prov->m_stalePlaylist );
    }

    Meta::TrackList orphanedTracks = findOrphanedTracks( knownPaths );
    if( !orphanedTracks.isEmpty() )
    {
        prov->m_orphanedPlaylist = Playlists::PlaylistPtr( new IpodPlaylist( orphanedTracks,
            i18nc( "iPod playlist name", "Orphaned tracks" ), m_coll, IpodPlaylist::Orphaned ) );
        prov->m_playlists << prov->m_orphanedPlaylist;  // we dont subscribe to this playlist, no need to update database
        emit prov->playlistAdded( prov->m_orphanedPlaylist );
    }

    if( !m_coll->m_itdb || m_aborted )
        return;
    for( GList *playlists = m_coll->m_itdb->playlists; playlists; playlists = playlists->next )
    {
        Itdb_Playlist *playlist = (Itdb_Playlist *) playlists->data;
        if( !playlist || itdb_playlist_is_mpl( playlist )  )
            continue; // skip null (?) or master playlists
        Playlists::PlaylistPtr playlistPtr( new IpodPlaylist( playlist, m_coll ) );
        prov->m_playlists << playlistPtr;
        prov->subscribeTo( playlistPtr );
        emit prov->playlistAdded( playlistPtr );
    }

    if( !m_aborted && ( prov->m_stalePlaylist || prov->m_orphanedPlaylist ) )
    {
        QString text = i18n( "Stale and/or orphaned tracks detected on %1. You can resolve "
            "the situation using the <b>%2</b> collection action. You can also view the tracks "
            "under the Saved Playlists tab.", m_coll->prettyName(),
            m_coll->m_consolidateAction->text() );
        Amarok::Components::logger()->longMessage( text );
    }
}

Meta::TrackList IpodParseTracksJob::findOrphanedTracks(const QSet< QString >& knownPaths)
{
    gchar *musicDirChar = itdb_get_music_dir( QFile::encodeName( m_coll->mountPoint() ) );
    QString musicDirPath = QFile::decodeName( musicDirChar );
    g_free( musicDirChar );
    musicDirChar = 0;

    QStringList trackPatterns;
    foreach( QString suffix, m_coll->supportedFormats() )
    {
        trackPatterns << QString( "*.%1" ).arg( suffix );
    }

    Meta::TrackList orphanedTracks;
    QDir musicDir( musicDirPath );
    foreach( QString subdir, musicDir.entryList( QStringList( "F??" ), QDir::Dirs | QDir::NoDotAndDotDot ) )
    {
        if( m_aborted )
            return Meta::TrackList();
        subdir = musicDir.absoluteFilePath( subdir ); // make the path absolute
        foreach( QFileInfo info, QDir( subdir ).entryInfoList( trackPatterns ) )
        {
            QString canonPath = info.canonicalFilePath();
            if( knownPaths.contains( canonPath ) )
                continue;  // already in iTunes database
            Meta::TrackPtr track( new MetaFile::Track( KUrl( canonPath ) ) );
            orphanedTracks << track;
        }
    }
    return orphanedTracks;
}