/*
 *  Copyright (c) 2009 Maximilian Kossick <maximilian.kossick@googlemail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "MemoryQueryMakerHelper.h"

#include <QList>
#include <QSet>
#include <QStack>
#include <QtAlgorithms>

#include <KRandomSequence>
#include <KSortableList>

template <class PointerType>
QList<PointerType>
MemoryQueryMakerHelper::orderListByName( const QList<PointerType> &list, qint64 value, bool descendingOrder )
{
    QList<PointerType> resultList = list;
    KSortableList<PointerType, QString> sortList;
    foreach( PointerType pointer, list )
    {
        sortList.insert( pointer->name(), pointer );
    }
    sortList.sort();
    QList<PointerType> tmpList;
    typedef KSortableItem<PointerType,QString> SortItem;
    foreach( SortItem item, sortList )
    {
       tmpList.append( item.second );
    }
    if( descendingOrder )
    {
        //KSortableList uses qSort, which orders a list in ascending order
        resultList = reverse<PointerType>( tmpList );
    }
    else
    {
        resultList = tmpList;
    }
    return resultList;
}

Meta::YearList
MemoryQueryMakerHelper::orderListByYear( const Meta::YearList &list, bool descendingOrder )
{
    KSortableList<Meta::YearPtr, double> sortList;
    foreach( Meta::YearPtr pointer, list )
    {
        sortList.insert( pointer->name().toDouble(), pointer );
    }
    sortList.sort();
    QList<Meta::YearPtr> tmpList;
    typedef KSortableItem<Meta::YearPtr,double> SortItem;
    foreach( SortItem item, sortList )
    {
        tmpList.append( item.second );
    }
    if( descendingOrder )
    {
        //KSortableList uses qSort, which orders a list in ascending order
        return reverse<Meta::YearPtr>( tmpList );
    }
    else
    {
        return tmpList;
    }
}

template<typename T>
QList<T>
MemoryQueryMakerHelper::reverse(const QList<T> &l)
{
    QList<T> ret;
    for (int i=l.size() - 1; i>=0; --i)
        ret.append(l.at(i));
    return ret;
}
