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


#define DEBUG_PREFIX "GpodderProvider"

#include "GpodderProvider.h"
#include "playlistmanager/PlaylistManager.h"
#include "NetworkAccessManagerProxy.h"
#include "core/support/Debug.h"

#include <kconfiggroup.h>
#include <StatusBar.h>

#include <QAction>

using namespace Podcasts;

GpodderProvider::GpodderProvider( const QString &username, const QString &password) : m_apiRequest(username,password,The::networkAccessManager()), m_username(username), m_timestamp(0), m_channels(), m_result(), m_removeAction(0), m_addList(), m_removeList(), m_timer(new QTimer(this))
{

    DEBUG_BLOCK

    m_updates = m_apiRequest.deviceUpdates(m_username,"amarok",m_timestamp);
    connect(m_updates.data(),SIGNAL(finished()), SLOT(finished()) );
    connect(m_updates.data(),SIGNAL(requestError(QNetworkReply::NetworkError)), SLOT(requestError(QNetworkReply::NetworkError)));
    connect(m_updates.data(),SIGNAL(parseError()), SLOT(parseError()));
    //The::statusBar()->longMessage("Loading playlist from gpodder",StatusBar::Information);                 //TODO i18n
    /*Add the provider for gpodder*/
    The::playlistManager()->addProvider(this,PlaylistManager::PodcastChannel);

    connect(m_timer,SIGNAL(timeout()),SLOT(timerUpdate()));
    m_timer->start(1000 * 60 * 3);
}

GpodderProvider::~GpodderProvider()
{
    DEBUG_BLOCK
    
    delete m_timer;
    //send remaining changes
    if (!m_removeList.isEmpty() || !m_addList.isEmpty())
    {
        m_result = m_apiRequest.addRemoveSubscriptions(m_username,"amarok",m_addList,m_removeList);
        m_addList.clear();
        m_removeList.clear();
    }
    /*Remove the provider*/
    The::playlistManager()->removeProvider(this);
}


void GpodderProvider::timerUpdate()
{
    DEBUG_BLOCK
    
    debug() << "add: " << m_addList.size();
    debug() << "remove: " << m_removeList.size();
    if (!m_removeList.isEmpty() || !m_addList.isEmpty())
    {
        m_result = m_apiRequest.addRemoveSubscriptions(m_username,"amarok",m_addList,m_removeList);     //TODO connect with slots and retry if request failed or inform user
        m_addList.clear();
        m_removeList.clear();
    }
}


bool Podcasts::GpodderProvider::possiblyContainsTrack(const KUrl& url) const
{
    DEBUG_BLOCK
    foreach(GpodderPodcastChannelPtr ptr, m_channels)
    {
        foreach(PodcastEpisodePtr episode, ptr->episodes()) {
            if (episode->uidUrl() == url.url()) return true;
        }
    }
    return false;
}

Meta::TrackPtr GpodderProvider::trackForUrl(const KUrl& url)
{
    DEBUG_BLOCK
    if (url.isEmpty()) return Meta::TrackPtr();
    foreach(GpodderPodcastChannelPtr podcast, m_channels)
    {
        foreach(PodcastEpisodePtr episode, podcast->episodes())
        {
            if (episode->uidUrl() == url.url())
            {
                return Meta::TrackPtr::dynamicCast( episode );
            }
        }
    }
    return Meta::TrackPtr();
}

PodcastEpisodePtr GpodderProvider::episodeForGuid(const QString& guid)
{
    foreach(GpodderPodcastChannelPtr ptr, m_channels)
    {
        foreach(PodcastEpisodePtr episode, ptr->episodes()) {
            if (episode->guid() == guid) return episode;
        }
    }
    return PodcastEpisodePtr();
}

void GpodderProvider::addPodcast(const KUrl& url)
{
    DEBUG_BLOCK
    
    m_addList << QUrl(url.url());
    PodcastChannelPtr p = PodcastChannelPtr(new PodcastChannel());
    p->setUrl(url);
    GpodderPodcastChannelPtr channel = GpodderPodcastChannelPtr(new GpodderPodcastChannel(this,p));
    readChannel(channel);
}


Playlists::PlaylistPtr GpodderProvider::addPlaylist(Playlists::PlaylistPtr playlist)
{
    DEBUG_BLOCK
    
    PodcastChannelPtr channel = PodcastChannelPtr::dynamicCast( playlist );
    if ( channel.isNull() )
        return Playlists::PlaylistPtr();
    GpodderPodcastChannelPtr gpodderChannel(new GpodderPodcastChannel(this,channel));
    readChannel(gpodderChannel);
    debug() << gpodderChannel->episodes().size();
    m_addList << QUrl(channel->url().url());
    return Playlists::PlaylistPtr::dynamicCast( addChannel( channel ) );
}


PodcastChannelPtr GpodderProvider::addChannel(PodcastChannelPtr channel)
{
    DEBUG_BLOCK
    
    GpodderPodcastChannelPtr gpodderChannel(new GpodderPodcastChannel(this,channel));
    //readChannel(gpodderChannel);
    //debug() << gpodderChannel->episodes().size();
    //m_addList << QUrl(channel->url().url());
    return PodcastChannelPtr::dynamicCast(gpodderChannel);
}

PodcastEpisodePtr GpodderProvider::addEpisode(PodcastEpisodePtr episode)
{
    if (episode.isNull())
        return PodcastEpisodePtr();

    if (episode->channel().isNull())
    {
        debug() << "channel is null";
        return PodcastEpisodePtr();
    }
    //TODO send new status of episode on gpodder.net
    return episode;
}

PodcastChannelList GpodderProvider::channels()
{
    DEBUG_BLOCK
    PodcastChannelList list;
    QListIterator<GpodderPodcastChannelPtr> i( m_channels );
    while ( i.hasNext() )
    {
        list << PodcastChannelPtr::dynamicCast( i.next() );
    }
    return list;
}

QString GpodderProvider::prettyName() const
{
    return i18n("Gpodder Podcasts");
}

KIcon GpodderProvider::icon() const
{
    return KIcon( "view-services-gpodder-amarok" );
}

Playlists::PlaylistList GpodderProvider::playlists()
{
    Playlists::PlaylistList playlist;
    foreach(GpodderPodcastChannelPtr channel, m_channels) {
        playlist << Playlists::PlaylistPtr::staticCast(channel);
        //debug() << channel->episodes().size();
    }
    return playlist;
}

void GpodderProvider::completePodcastDownloads()
{

}

void GpodderProvider::finished()
{
    DEBUG_BLOCK
    m_timestamp = m_updates->timestamp();

    /*foreach(QUrl url, m_updates->removeList()) {
        debug() << "Removing channel " << url;
        removeChannel(url);
    }*/

    foreach(mygpo::PodcastPtr podcast, m_updates->addList()) {
        debug() << "Adding channel " << podcast->url();
        GpodderPodcastChannelPtr channel(new GpodderPodcastChannel(this,toPodcastChannelPtr(podcast)));
        readChannel(channel);
    }
    //The::statusBar()->longMessage("Playlist loaded successfully",StatusBar::Information);  //TODO i18n
}

void GpodderProvider::parseError()
{
    DEBUG_BLOCK
    //The::statusBar()->longMessage("Parse error",StatusBar::Error);               //TODO i18n
}

void GpodderProvider::requestError(QNetworkReply::NetworkError error)
{
    DEBUG_BLOCK
    debug() << "Request error nr.: " << error;
    //The::statusBar()->longMessage(QString("Request error: ")+QString::number(error),StatusBar::Error);     //TODO i18n
}



PodcastChannelPtr GpodderProvider::toPodcastChannelPtr(mygpo::PodcastPtr podcast)
{
    DEBUG_BLOCK

    GpodderPodcastChannelPtr p(new GpodderPodcastChannel(this));
    p->setUrl(podcast->url());
    p->setWebLink(podcast->website());
    p->setImageUrl(podcast->logoUrl());
    p->setDescription(podcast->description());
    p->setTitle(podcast->title());
    return Podcasts::PodcastChannelPtr::dynamicCast( p );
}

void GpodderProvider::readChannel(GpodderPodcastChannelPtr channel)
{
    DEBUG_BLOCK

    PodcastReader *podcastReader = new PodcastReader( this );
    
    connect( podcastReader, SIGNAL( finished( PodcastReader * ) ),
             SLOT( slotReadResult( PodcastReader * ) ) );
    connect( podcastReader, SIGNAL( statusBarSorryMessage( const QString & ) ),
             this, SLOT( slotStatusBarSorryMessage( const QString & ) ) );
    connect( podcastReader, SIGNAL( statusBarNewProgressOperation( KIO::TransferJob *, const QString &, Podcasts::PodcastReader* ) ),
             this, SLOT( slotStatusBarNewProgressOperation( KIO::TransferJob *, const QString &, Podcasts::PodcastReader* ) ) );

    //podcastReader->update( Podcasts::PodcastChannelPtr::dynamicCast( channel ) );
    podcastReader->read(channel->url());
}

void GpodderProvider::slotReadResult(PodcastReader* podcastReader)
{
    DEBUG_BLOCK

    if ( podcastReader->error() != QXmlStreamReader::NoError )
    {
        debug() << podcastReader->errorString();
        //The::statusBar()->longMessage( podcastReader->errorString(), StatusBar::Error );
    }

    Podcasts::PodcastChannelPtr channel = podcastReader->channel();

    if (channel.isNull()) {
        return;
    }
    addPodcastChannel(channel);
    podcastReader->deleteLater();
}

void GpodderProvider::slotStatusBarSorryMessage( const QString &message )
{
    DEBUG_BLOCK
    //The::statusBar()->longMessage( message, StatusBar::Sorry );
}

void GpodderProvider::slotStatusBarNewProgressOperation( KIO::TransferJob * job,
        const QString &description,
        Podcasts::PodcastReader* reader )
{
    DEBUG_BLOCK
    //The::statusBar()->newProgressOperation( job, description )->setAbortSlot( reader, SLOT( slotAbort() ) );
}

void GpodderProvider::addPodcastChannel(PodcastChannelPtr channel)
{
    DEBUG_BLOCK
    GpodderPodcastChannelPtr c = GpodderPodcastChannelPtr(new GpodderPodcastChannel(this,channel));
    m_channels << c;
    //debug() << channel->episodes().size();
    emit playlistAdded(Playlists::PlaylistPtr::dynamicCast( c ));
}

void GpodderProvider::removeChannel(const QUrl& url)
{
    for (int i = 0; i < m_channels.size(); i++) {
        if (m_channels.at(i)->url() == url) {
            m_channels.removeAt(i);
            return;
        }
    }
}

QList<QAction *> GpodderProvider::channelActions( PodcastChannelList channels)
{
    DEBUG_BLOCK
    QList<QAction *> actions;
    if ( m_removeAction == 0 )
    {
        m_removeAction = new QAction(
            KIcon( "edit-delete" ),
            i18n( "&Delete Channel and Episodes" ),
            this
        );
        m_removeAction->setProperty( "popupdropper_svg_id", "delete" );
        connect( m_removeAction, SIGNAL( triggered() ),
                 SLOT( slotRemoveChannels() ) );
    }
    //set the episode list as data that we'll retrieve in the slot
    PodcastChannelList actionList =
        m_removeAction->data().value<PodcastChannelList>();

    actionList << channels;
    m_removeAction->setData( QVariant::fromValue( actionList ) );

    actions << m_removeAction;
    return actions;
}

QList<QAction *> GpodderProvider::playlistActions( Playlists::PlaylistPtr playlist )
{
    DEBUG_BLOCK
    
    PodcastChannelList channels;
    PodcastChannelPtr channel = PodcastChannelPtr::dynamicCast( playlist );
    if ( channel.isNull() )
        return QList<QAction *>();

    return channelActions( channels << channel );

}

void GpodderProvider::slotRemoveChannels()
{
    DEBUG_BLOCK
    QAction *action = qobject_cast<QAction *>( QObject::sender() );
    if ( action == 0 )
        return;

    PodcastChannelList channels = action->data().value<PodcastChannelList>();
    action->setData( QVariant() );      //Clear data

    foreach( PodcastChannelPtr channel, channels)
    {
        QUrl url(channel->url().url());
        m_removeList << url;
        removeChannel(url);
        emit playlistRemoved( Playlists::PlaylistPtr::dynamicCast( channel ) );
    }
}

