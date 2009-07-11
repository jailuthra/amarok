/****************************************************************************************
 * Copyright (c) 2009  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Pulic License for more details.              *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/
 
#ifndef CURRENTTRACKTOOLBAR_H
#define CURRENTTRACKTOOLBAR_H

#include "EngineObserver.h" //baseclass

#include <QToolBar>

/**
A toolbar that contains the CurrentTrackActions of the currently playing track.

	@author Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>
*/
class CurrentTrackToolbar : public QToolBar, public EngineObserver
{
public:
    CurrentTrackToolbar( QWidget * parent );

    ~CurrentTrackToolbar();

    virtual void engineStateChanged( Phonon::State state, Phonon::State oldState = Phonon::StoppedState );
    virtual void engineNewMetaData( const QHash<qint64, QString> &newMetaData, bool trackChanged );

protected:
    void handleAddActions();

};

#endif
