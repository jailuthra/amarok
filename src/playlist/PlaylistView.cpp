/***************************************************************************
 * copyright            : (C) 2007 Ian Monroe <ian@monroe.nu>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2        *
 *   as published by the Free Software Foundation.                         *
 ***************************************************************************/

#include "PlaylistDelegate.h"
#include "PlaylistView.h"

#include <QItemDelegate>

using namespace PlaylistNS;

void
View::setModel( QAbstractItemModel * model )
{
    QListView::setModel( model );
    setDropIndicatorShown( true );
    setSelectionMode( QAbstractItemView::ExtendedSelection );
    setSelectionBehavior( QAbstractItemView::SelectRows );
    setDragDropMode( QAbstractItemView::DragDrop );
    setDragDropOverwriteMode( true );
    setDragEnabled( true );
    setAcceptDrops( true );
    setDropIndicatorShown( true );
    setAlternatingRowColors( true );
    //setMovement( QListView::Free );
    delete itemDelegate();
    setItemDelegate( new Delegate( this ) );
    connect( this, SIGNAL( activated( const QModelIndex& ) ), model, SLOT( play( const QModelIndex& ) ) );
}
#include "PlaylistView.moc"
