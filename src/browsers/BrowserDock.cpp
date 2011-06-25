/****************************************************************************************
 * Copyright (c) 2009 Nikolaj Hald Nielsen <nhn@kde.org>                                *
 * Copyright (c) 2009 Mark Kretschmann <kretschmann@kde.org>                            *
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
 
#include "BrowserDock.h"

#include "App.h"
#include "core/support/Amarok.h"
#include "core/support/Debug.h"
#include "widgets/HorizontalDivider.h"
#include "PaletteHandler.h"
#include "statusbar/StatusBar.h"

#include <KAction>
#include <KIcon>
#include <KLocale>

#include <QWidget>

BrowserDock::BrowserDock( QWidget *parent )
    : AmarokDockWidget( i18n( "&Media Sources" ), parent )
{
    setObjectName( "Media Sources dock" );
    setAllowedAreas( Qt::AllDockWidgetAreas );

    //we have to create this here as it is used when setting up the
    //categories (unless of course we move that to polish as well...)
    m_mainWidget = new KVBox( this );
    setWidget( m_mainWidget );
    m_mainWidget->setContentsMargins( 0, 0, 0, 0 );
    m_mainWidget->setFrameShape( QFrame::NoFrame );
    m_mainWidget->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Ignored );
    m_mainWidget->setFocus( Qt::ActiveWindowFocusReason );

    m_breadcrumbWidget = new BrowserBreadcrumbWidget( m_mainWidget );
    new HorizontalDivider( m_mainWidget );
    m_categoryList = new BrowserCategoryList( m_mainWidget, "root list" );
    m_breadcrumbWidget->setRootList( m_categoryList.data() );

    //HACK: keep the progressArea in it's place at the bottom
    QWidget *container = new QWidget( m_mainWidget );
    container->setLayout( new QVBoxLayout( container ) );
    container->layout()->setContentsMargins( 0, 0, 0, 0 );
    container->layout()->setSpacing( 0 );
    m_progressFrame = The::statusBar()->progressArea();
    m_progressFrame->setAutoFillBackground( true );
    m_progressFrame->setFixedHeight( 30 );
    container->layout()->addWidget( m_progressFrame );

    ensurePolish();
}

BrowserDock::~BrowserDock()
{}

void BrowserDock::polish()
{
    m_categoryList.data()->setIcon( KIcon( "user-home" ) );

    m_categoryList.data()->setMinimumSize( 100, 300 );

    connect( m_breadcrumbWidget, SIGNAL( toHome() ), this, SLOT( home() ) );

    // Keyboard shortcut for going back one level
    KAction *action = new KAction( KIcon( "go-previous" ), i18n( "Previous Browser" ),
                                  m_mainWidget );
    Amarok::actionCollection()->addAction( "browser_previous", action );
    connect( action, SIGNAL(triggered( bool )), m_categoryList.data(), SLOT(back()) );
    action->setShortcut( KShortcut( Qt::CTRL + Qt::Key_Left ) );

    paletteChanged( App::instance()->palette() );
    connect( The::paletteHandler(), SIGNAL(newPalette( const QPalette & )),
             SLOT(paletteChanged( const QPalette & )) );
}

BrowserCategoryList *BrowserDock::list() const
{
    return m_categoryList.data();
}

void
BrowserDock::navigate( const QString &target )
{
    m_categoryList.data()->navigate( target );
}

void
BrowserDock::home()
{
    m_categoryList.data()->home();
}

void
BrowserDock::paletteChanged( const QPalette &palette )
{
    Q_UNUSED(palette); //palette is accessible via PaletteHandler
    m_progressFrame->setStyleSheet(
                QString( "QFrame { background-color: %1; color: %2; border-radius: 3px; }" )
                        .arg( PaletteHandler::alternateBackgroundColor().name() )
                        .arg( The::paletteHandler()->palette().highlightedText().color().name() )
                );
}

#include "BrowserDock.moc"
