/***************************************************************************
 *   Copyright (c) 2008  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
 
#include "SqlPlaylistGroup.h"

#include "CollectionManager.h"
#include "Debug.h"
#include "meta/SqlPlaylist.h"
#include "SqlStorage.h"

SqlPlaylistGroup::SqlPlaylistGroup( const QStringList & dbResultRow, SqlPlaylistGroup * parent )
    : SqlPlaylistViewItem()
    , m_parent( parent )
    ,  m_hasFetchedChildGroups( false )
    , m_hasFetchedChildPlaylists( false )
{
    m_dbId = dbResultRow[0].toInt();
    m_name = dbResultRow[2];
    m_description = dbResultRow[3];

}

SqlPlaylistGroup::SqlPlaylistGroup( const QString & name, SqlPlaylistGroup * parent )
    : SqlPlaylistViewItem()
    , m_dbId( -1 )
    , m_parent( parent )
    , m_name( name )
    , m_description( QString() )
    , m_hasFetchedChildGroups( false )
    , m_hasFetchedChildPlaylists( false )
{

    if ( parent == 0 ) {
        //root item
        m_dbId = 0;
    }
}


SqlPlaylistGroup::~SqlPlaylistGroup()
{
    clear();
}

void SqlPlaylistGroup::save()
{
    int parentId = 0;
    if ( m_parent )
        parentId = m_parent->id();
    
    if ( m_dbId != -1 ) {
        //update existing
        QString query = "UPDATE playlist_groups SET parent_id=%1, name='%2', description='%3' WHERE id=%4;";
        query = query.arg( QString::number( parentId ) ).arg( m_name ).arg( m_description ).arg( QString::number( m_dbId ) );
        CollectionManager::instance()->sqlStorage()->query( query );
        
    } else {
        //insert new
        
        QString query = "INSERT INTO playlist_groups ( parent_id, name, description) VALUES ( %1, %2, %3 );";
        query = query.arg( QString::number( parentId ) ).arg( m_name ).arg( m_description );
        m_dbId = CollectionManager::instance()->sqlStorage()->insert( query, NULL );

    }

    
}

SqlPlaylistGroupList SqlPlaylistGroup::childGroups()
{
    //DEBUG_BLOCK
    if ( !m_hasFetchedChildGroups ) {

        QString query = "SELECT id, parent_id, name, description FROM playlist_groups where parent_id=%1;";
        query = query.arg( QString::number( m_dbId ) );
        QStringList result = CollectionManager::instance()->sqlStorage()->query( query );


        int resultRows = result.count() / 4;

        for( int i = 0; i < resultRows; i++ )
        {
            QStringList row = result.mid( i*4, 4 );
            m_childGroups << new SqlPlaylistGroup( row, this );
        }

        m_hasFetchedChildGroups = true;

    }

    return m_childGroups;
    
}

SqlPlaylistDirectList SqlPlaylistGroup::childPlaylists()
{
    DEBUG_BLOCK
    debug() << "my name: " << m_name << " my pointer: " << this;
    if ( !m_hasFetchedChildPlaylists ) {

        QString query = "SELECT id, parent_id, name, description FROM playlists where parent_id=%1;";
        query = query.arg( QString::number( m_dbId ) );
        QStringList result = CollectionManager::instance()->sqlStorage()->query( query );

        debug() << "Result: " << result;


        int resultRows = result.count() / 4;

        for( int i = 0; i < resultRows; i++ )
        {
            QStringList row = result.mid( i*4, 4 );
            m_childPlaylists << new Meta::SqlPlaylist( row, this );
        }

        m_hasFetchedChildPlaylists = true;

    }

    return m_childPlaylists;
}

int SqlPlaylistGroup::id()
{
    return m_dbId;
}

QString SqlPlaylistGroup::name() const
{
    return m_name;
}

QString SqlPlaylistGroup::description() const
{
    return m_description;
}

int SqlPlaylistGroup::childCount()
{
    return childGroups().count() + childPlaylists().count();
}

void SqlPlaylistGroup::clear()
{
    while( !m_childGroups.isEmpty() )
        delete m_childGroups.takeFirst();
    while( !m_childPlaylists.isEmpty() )
        delete m_childPlaylists.takeFirst();

    m_hasFetchedChildGroups = false;
    m_hasFetchedChildPlaylists = false;
}




