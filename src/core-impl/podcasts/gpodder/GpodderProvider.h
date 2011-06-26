/****************************************************************************************
 * Copyright (c) 2011 Stefan Derkits <stefan@derkits.at>                                *
 * Copyright (c) 2011 Christian Wagner <christian.wagner86@gmx.at>                      *
 * Copyright (c) 2011 Felix Winter <ixos01@gmail.com>                                   *
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


#ifndef GPODDERPODCASTPROVIDER_H
#define GPODDERPODCASTPROVIDER_H

#include "core/podcasts/PodcastProvider.h"

#include <mygpo-qt/ApiRequest.h>
#include "core/podcasts/PodcastReader.h"
#include "GpodderPodcastMeta.h"

#include <KLocale>
#include <KDialog>

class QAction;

namespace Podcasts {

class GpodderProvider : public PodcastProvider
{
    Q_OBJECT
public:

    GpodderProvider(const QString& username, const QString& password);
    ~GpodderProvider();

    bool possiblyContainsTrack( const KUrl &url ) const;
    Meta::TrackPtr trackForUrl( const KUrl &url );
    /** Special function to get an episode for a given guid.
     *
     * note: this functions is required because KUrl does not preserve every possible guids.
     * This means we can not use trackForUrl().
     * Problematic guids contain non-latin characters, percent encoded parts, capitals, etc.
     */
    virtual PodcastEpisodePtr episodeForGuid( const QString &guid );

    virtual void addPodcast( const KUrl &url );

    virtual Podcasts::PodcastChannelPtr addChannel( Podcasts::PodcastChannelPtr channel );
    virtual Podcasts::PodcastEpisodePtr addEpisode( Podcasts::PodcastEpisodePtr episode );

    virtual Podcasts::PodcastChannelList channels();

    // PlaylistProvider methods
    virtual QString prettyName() const;
    virtual KIcon icon() const;
    virtual Playlists::PlaylistList playlists();
    virtual void completePodcastDownloads();

    /** Copy a playlist to the provider.
    */
    virtual Playlists::PlaylistPtr addPlaylist( Playlists::PlaylistPtr playlist );
    QList<QAction *> channelActions( PodcastChannelList episodes );
    QList<QAction *> playlistActions( Playlists::PlaylistPtr playlist );
    
signals:
    //PlaylistProvider signals
    void updated();
    void playlistAdded( Playlists::PlaylistPtr playlist );
    void playlistRemoved( Playlists::PlaylistPtr playlist );

private slots:
    void finished();
    void parseError();
    void requestError( QNetworkReply::NetworkError error );
    void slotReadResult( PodcastReader *podcastReader );
    void slotStatusBarSorryMessage( const QString &message );
    void slotStatusBarNewProgressOperation( KIO::TransferJob * job,
                                                       const QString &description,
                                                       Podcasts::PodcastReader* reader );
    void slotRemoveChannels();
    void timerUpdate();

    void slotSyncPlaylistAdded( Playlists::PlaylistPtr playlist );
    void slotSyncPlaylistRemoved( Playlists::PlaylistPtr playlist );

    void slotSyncPlaylistAddedDialog();
    void slotSyncPlaylistRemovedDialog();
    
private:
    mygpo::ApiRequest m_apiRequest;
    const QString m_username;
    mygpo::DeviceUpdatesPtr m_updates;
    qulonglong m_timestamp;
    GpodderPodcastChannelList m_channels;
    mygpo::AddRemoveResultPtr m_result;

    KDialog *m_askDiag;
    Playlists::PlaylistPtr m_possibleSyncPlaylist;
    
    void readChannel( GpodderPodcastChannelPtr channel );
    PodcastChannelPtr toPodcastChannelPtr( mygpo::PodcastPtr podcast );
    void addPodcastChannel( PodcastChannelPtr channel );
    void removeChannel( const QUrl& url );
    PodcastChannelPtr initializeMaster( PodcastChannelPtr channel );
    PodcastChannelPtr initializeSlave( PodcastChannelPtr channel );

    QAction *m_removeAction; //remove a subscription
    QList<QUrl> m_addList;
    QList<QUrl> m_removeList;

    QTimer *m_timer;
};

}


#endif
