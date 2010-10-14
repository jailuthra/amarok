/****************************************************************************************
 * Copyright (c) 2010 Maximilian Kossick <maximilian.kossick@googlemail.com>       *
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

#include "TestMemoryQueryMaker.h"

#include "core/support/Debug.h"

#include "core/collections/QueryMaker.h"
#include "core/meta/Meta.h"
#include "core-impl/collections/support/MemoryCollection.h"
#include "core-impl/collections/support/MemoryQueryMaker.h"
#include "core-impl/collections/support/MemoryFilter.h"

#include "mocks/MetaMock.h"
#include "mocks/MockTrack.h"

#include <QVariantMap>
#include <QSharedPointer>
#include <QSignalSpy>

#include <KCmdLineArgs>
#include <KGlobal>

#include <qtest_kde.h>

#include <gmock/gmock.h>

using ::testing::AnyNumber;
using ::testing::Return;

QTEST_KDEMAIN_CORE( TestMemoryQueryMaker )

TestMemoryQueryMaker::TestMemoryQueryMaker()
{
    KCmdLineArgs::init( KGlobal::activeComponent().aboutData() );
    ::testing::InitGoogleMock( &KCmdLineArgs::qtArgc(), KCmdLineArgs::qtArgv() );
    qRegisterMetaType<Meta::TrackList>();
    qRegisterMetaType<Meta::AlbumList>();
    qRegisterMetaType<Meta::ArtistList>();
}

void
TestMemoryQueryMaker::testDeleteQueryMakerWhileQueryIsRunning()
{
    QSharedPointer<Collections::MemoryCollection> mc( new Collections::MemoryCollection() );
    mc->addTrack( Meta::TrackPtr( new MetaMock( QVariantMap() )));
    mc->addTrack( Meta::TrackPtr( new MetaMock( QVariantMap() )));
    Meta::MockTrack *mock = new Meta::MockTrack();
    EXPECT_CALL( *mock, uidUrl() ).Times( AnyNumber() ).WillRepeatedly( Return( "track3" ) );
    Meta::TrackPtr trackPtr( mock );
    mc->addTrack( trackPtr );

    Collections::MemoryQueryMaker *qm = new Collections::MemoryQueryMaker( mc.toWeakRef(), "test" );
    qm->setQueryType( Collections::QueryMaker::Track );

    qm->run();
    delete qm;
    //we cannot wait for a signal here....
    //QTest::qWait( 500 );
}

void
TestMemoryQueryMaker::testDeleteCollectionWhileQueryIsRunning()
{
    QSharedPointer<Collections::MemoryCollection> mc( new Collections::MemoryCollection() );
    mc->addTrack( Meta::TrackPtr( new MetaMock( QVariantMap() )));
    mc->addTrack( Meta::TrackPtr( new MetaMock( QVariantMap() )));

    Collections::MemoryQueryMaker *qm = new Collections::MemoryQueryMaker( mc, "test" );
    qm->setQueryType( Collections::QueryMaker::Track );

    QSignalSpy spy( qm, SIGNAL(queryDone()));

    qm->run();
    mc.clear();
    QTest::qWait( 500 );
    QCOMPARE( spy.count(), 1 );

    delete qm;
}

class TestStringMemoryFilter : public StringMemoryFilter
{
public:
    TestStringMemoryFilter() : StringMemoryFilter() {}

protected:
    QString value( const Meta::TrackPtr &track ) const { return "abcdef"; }

};

void
TestMemoryQueryMaker::testStringMemoryFilterSpeedFullMatch()
{
    //Test 1: match complete string
    TestStringMemoryFilter filter1;
    filter1.setFilter( QString( "abcdef" ), true, true );

    QBENCHMARK {
        filter1.filterMatches( Meta::TrackPtr() );
    }
}

void
TestMemoryQueryMaker::testStringMemoryFilterSpeedMatchBegin()
{
    //Test 2: match beginning of string
    TestStringMemoryFilter filter2;
    filter2.setFilter( QString( "abcd" ), true, false );

    QBENCHMARK {
        filter2.filterMatches( Meta::TrackPtr() );
    }
}

void
TestMemoryQueryMaker::testStringMemoryFilterSpeedMatchEnd()
{
    //Test 3: match end of string
    TestStringMemoryFilter filter3;
    filter3.setFilter( QString( "cdef" ), false, true );

    QBENCHMARK {
        filter3.filterMatches( Meta::TrackPtr() );
    }
}

void
TestMemoryQueryMaker::testStringMemoryFilterSpeedMatchAnywhere()
{
    //Test 4: match anywhere in string
    TestStringMemoryFilter filter4;
    filter4.setFilter( QString( "bcde" ), false, false );

    QBENCHMARK {
        filter4.filterMatches( Meta::TrackPtr() );
    }
}
