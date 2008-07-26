/******************************************************************************
 * Copyright (C) 2008 Peter ZHOU <peterzhoulei@gmail.com>                     *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License as             *
 * published by the Free Software Foundation; either version 2 of             *
 * the License, or (at your option) any later version.                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
 ******************************************************************************/

#include "AmarokStatusbarScript.h"

#include "App.h"
#include "statusbar/StatusBar.h"

#include <QtScript>

namespace AmarokScript
{
    AmarokStatusbarScript::AmarokStatusbarScript( QScriptEngine* ScriptEngine )
    : QObject( kapp )
    {
        Q_UNUSED( ScriptEngine );
    }

    AmarokStatusbarScript::~AmarokStatusbarScript()
    {

    }

    void AmarokStatusbarScript::setMainText( const QString &text )
    {
        Amarok::StatusBar::instance()->setMainText( text );
    }

    void AmarokStatusbarScript::setMainTextIcon( QPixmap icon )
    {
        Amarok::StatusBar::instance()->setMainTextIcon( icon );
    }

    void AmarokStatusbarScript::hideMainTextIcon()
    {
        Amarok::StatusBar::instance()->hideMainTextIcon();
    }

    void AmarokStatusbarScript::resetMainText()
    {
        Amarok::StatusBar::instance()->resetMainText();
    }

    void AmarokStatusbarScript::longMessage( const QString &text )
    {
        Amarok::StatusBar::instance()->longMessage( text );
    }

    void AmarokStatusbarScript::shortLongMessage( const QString &_short, const QString &_long )
    {
        Amarok::StatusBar::instance()->shortLongMessage( _short, _long );
    }

    void AmarokStatusbarScript::shortMessage( const QString &text, bool longShort )
    {
        Amarok::StatusBar::instance()->shortMessage( text, longShort );
    }
}

#include "AmarokStatusbarScript.moc"
