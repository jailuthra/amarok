/****************************************************************************************
 * Copyright (c) 2010 Sergey Ivanov <123kash@gmail.com>                                 *
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

#include "TagHelper.h"

#include <QRegExp>
#include <QStringList>

#include <fileref.h>
#include <aifffile.h>
#include <asffile.h>
#include <flacfile.h>
#include <mp4file.h>
#include <mpcfile.h>
#include <mpegfile.h>
#include <oggfile.h>
#include <oggflacfile.h>
#include <rifffile.h>
#include <speexfile.h>
#include <trueaudiofile.h>
#include <vorbisfile.h>
#include <wavfile.h>
#include <wavpackfile.h>

#include "APETagHelper.h"
#include "ASFTagHelper.h"
#include "ID3v2TagHelper.h"
#include "MP4TagHelper.h"
#include "VorbisCommentTagHelper.h"

#include "StringHelper.h"

using namespace Meta::Tag;

TagHelper::TagHelper( TagLib::Tag *tag, Amarok::FileType fileType )
         : m_tag( tag )
         , m_fileType( fileType )
{
}

TagHelper::TagHelper( TagLib::ID3v1::Tag *tag, Amarok::FileType fileType )
         : m_tag( tag )
         , m_fileType( fileType )
{
}

Meta::FieldHash
TagHelper::tags() const
{
    Meta::FieldHash data;
    TagLib::String str;
    TagLib::uint nmbr;

    if( !( str = m_tag->title() ).isEmpty() )
        data.insert( Meta::valTitle, TStringToQString( str ) );
    if( !( str = m_tag->artist() ).isEmpty() )
        data.insert( Meta::valArtist, TStringToQString( str ) );
    if( !( str = m_tag->album() ).isEmpty() )
        data.insert( Meta::valAlbum, TStringToQString( str ) );
    if( ( nmbr = m_tag->track() ) )
        data.insert( Meta::valTrackNr, nmbr );
    if( ( nmbr = m_tag->year() ) )
        data.insert( Meta::valYear, nmbr );
    if( !( str = m_tag->genre() ).isEmpty() )
        data.insert( Meta::valGenre, TStringToQString( str ) );
    if( !( str = m_tag->comment() ).isEmpty() )
        data.insert( Meta::valComment, TStringToQString( str ) );

    return data;
}

bool
TagHelper::setTags( const Meta::FieldHash &changes )
{
    bool modified = false;

    if( changes.contains( Meta::valTitle ) )
    {
        m_tag->setTitle( Qt4QStringToTString( changes.value( Meta::valTitle ).toString() ) );
        modified = true;
    }
    if( changes.contains( Meta::valArtist ) )
    {
        m_tag->setArtist( Qt4QStringToTString( changes.value( Meta::valArtist ).toString() ) );
        modified = true;
    }
    if( changes.contains( Meta::valAlbum ) )
    {
        m_tag->setAlbum( Qt4QStringToTString( changes.value( Meta::valAlbum ).toString() ) );
        modified = true;
    }
    if( changes.contains( Meta::valTrackNr ) )
    {
        m_tag->setTrack( changes.value( Meta::valTrackNr ).toUInt() );
        modified = true;
    }
    if( changes.contains( Meta::valYear ) )
    {
        m_tag->setYear( changes.value( Meta::valYear ).toUInt() );
        modified = true;
    }
    if( changes.contains( Meta::valGenre ) )
    {
        m_tag->setGenre( Qt4QStringToTString( changes.value( Meta::valGenre ).toString() ) );
        modified = true;
    }
    if( changes.contains( Meta::valComment ) )
    {
        m_tag->setGenre( Qt4QStringToTString( changes.value( Meta::valGenre ).toString() ) );
        modified = true;
    }

    return modified;
}

TagLib::ByteVector
TagHelper::render() const
{
    return TagLib::ByteVector();
}

#ifndef UTILITIES_BUILD
bool
TagHelper::hasEmbeddedCover() const
{
    return false;
}

QImage
TagHelper::embeddedCover() const
{
    return QImage();
}

bool
TagHelper::setEmbeddedCover( const QImage &cover )
{
    Q_UNUSED( cover )
    return false;
}
#endif  //UTILITIES_BUILD

TagLib::String
TagHelper::fieldName( const qint64 field ) const
{
    return m_fieldMap.value( field );
}

qint64
TagHelper::fieldName( const TagLib::String &field ) const
{
    return m_fieldMap.key( field );
}

QPair< TagHelper::UIDType, QString >
TagHelper::splitUID( const QString &uidUrl ) const
{
    TagHelper::UIDType type = UIDInvalid;
    QString uid = uidUrl;

    if( uid.startsWith( "amarok-" ) )
        uid = uid.remove( QRegExp( "^(amarok-\\w+://).+$" ) );

    if( uid.startsWith( "mb-" ) )
    {
        uid = uid.mid( 3 );
        if( isValidUID( uid, UIDMusicBrainz ) )
            type = UIDMusicBrainz;
    }
    else if( isValidUID( uid, UIDAFT ) )
        type = UIDAFT;

    return qMakePair( type, uid );
}

QPair< int, int >
TagHelper::splitDiscNr( const QString &value ) const
{
    int disc;
    int count = 0;
    if( value.indexOf( '/' ) != -1 )
    {
        QStringList list = value.split( '/', QString::SkipEmptyParts );
        disc = list.value( 0 ).toInt();
        count = list.value( 1 ).toInt();
    }
    else if( value.indexOf( ':' ) != -1 )
    {
        QStringList list = value.split( ':', QString::SkipEmptyParts );
        disc = list.value( 0 ).toInt();
        count = list.value( 1 ).toInt();
    }
    else
        disc = value.toInt();

    return qMakePair( disc, count );
}

bool
TagHelper::isValidUID( const QString &uid, const TagHelper::UIDType type ) const
{
    QRegExp regexp( "^$" );

    if( type == UIDAFT )
        regexp.setPattern( "^[0-9a-fA-F]{32}$" );
    else if( type == UIDMusicBrainz )
        regexp.setPattern( "^[0-9a-fA-F]{8}(-[0-9a-fA-F]{4}){3}-[0-9a-fA-F]{12}$" );

    return regexp.exactMatch( uid );
}

TagLib::String
TagHelper::uidFieldName( const TagHelper::UIDType type ) const
{
    return m_uidFieldMap.value( type );
}

TagLib::String
TagHelper::fmpsFieldName( const TagHelper::FMPS field ) const
{
    return m_fmpsFieldMap.value( field );
}

Amarok::FileType
TagHelper::fileType() const
{
    return m_fileType;
}

QByteArray
TagHelper::testString() const
{
    TagLib::String string = m_tag->album() + m_tag->artist() + m_tag->comment() +
                            m_tag->genre() + m_tag->title();

    return QByteArray( string.toCString( true ) );
}


Meta::Tag::TagHelper *
Meta::Tag::selectHelper( const TagLib::FileRef fileref, bool forceCreation )
{
    TagHelper *tagHelper = NULL;

    if( TagLib::MPEG::File *file = dynamic_cast< TagLib::MPEG::File * >( fileref.file() ) )
    {
        if( file->ID3v2Tag( forceCreation ) )
            tagHelper = new ID3v2TagHelper( file->ID3v2Tag(), Amarok::Mp3 );
        else if( file->APETag() )
            tagHelper = new APETagHelper( file->APETag(), Amarok::Mp3 );
        else if( file->ID3v1Tag() )
            tagHelper = new TagHelper( file->ID3v1Tag(), Amarok::Mp3 );
    }
    else if( TagLib::Ogg::Vorbis::File *file = dynamic_cast< TagLib::Ogg::Vorbis::File * >( fileref.file() ) )
    {
        if( file->tag() )
            tagHelper = new VorbisCommentTagHelper( file->tag(), Amarok::Ogg );
    }
    else if( TagLib::Ogg::FLAC::File *file = dynamic_cast< TagLib::Ogg::FLAC::File * >( fileref.file() ) )
    {
        if( file->tag() )
            tagHelper = new VorbisCommentTagHelper( file->tag(), Amarok::Ogg );
    }
    else if( TagLib::Ogg::Speex::File *file = dynamic_cast< TagLib::Ogg::Speex::File * >( fileref.file() ) )
    {
        if( file->tag() )
            tagHelper = new VorbisCommentTagHelper( file->tag(), Amarok::Ogg );
    }
    else if( TagLib::FLAC::File *file = dynamic_cast< TagLib::FLAC::File * >( fileref.file() ) )
    {
        if( file->xiphComment() )
            tagHelper = new VorbisCommentTagHelper( file->xiphComment(), Amarok::Flac );
        else if( file->ID3v2Tag() )
            tagHelper = new ID3v2TagHelper( file->ID3v2Tag(), Amarok::Flac );
        else if( file->ID3v1Tag() )
            tagHelper = new TagHelper( file->ID3v1Tag(), Amarok::Flac );
    }
    else if( TagLib::MP4::File *file = dynamic_cast< TagLib::MP4::File * >( fileref.file() ) )
    {
        TagLib::MP4::Tag *tag = dynamic_cast< TagLib::MP4::Tag * >( file->tag() );
        if( tag )
            tagHelper = new MP4TagHelper( tag, Amarok::Mp4 );
    }
    else if( TagLib::MPC::File *file = dynamic_cast< TagLib::MPC::File * >( fileref.file() ) )
    {
        if( file->APETag( forceCreation ) )
            tagHelper = new APETagHelper( file->APETag(), Amarok::Mpc );
        else if( file->ID3v1Tag() )
            tagHelper = new TagHelper( file->ID3v1Tag(), Amarok::Mpc );
    }
    else if( TagLib::RIFF::AIFF::File *file = dynamic_cast< TagLib::RIFF::AIFF::File * >( fileref.file() ) )
    {
        if( file->tag() )
            tagHelper = new ID3v2TagHelper( file->tag(), Amarok::Aiff );
    }
    else if( TagLib::RIFF::WAV::File *file = dynamic_cast< TagLib::RIFF::WAV::File * >( fileref.file() ) )
    {
        if( file->tag() )
            tagHelper = new ID3v2TagHelper( file->tag(), Amarok::Wav );
    }
    else if( TagLib::ASF::File *file = dynamic_cast< TagLib::ASF::File * >( fileref.file() ) )
    {
        TagLib::ASF::Tag *tag = dynamic_cast< TagLib::ASF::Tag * >( file->tag() );
        if( tag )
            tagHelper = new ASFTagHelper( tag, Amarok::Wma );
    }
    else if( TagLib::TrueAudio::File *file = dynamic_cast< TagLib::TrueAudio::File * >( fileref.file() ) )
    {
        if( file->ID3v2Tag( forceCreation ) )
            tagHelper = new ID3v2TagHelper( file->ID3v2Tag(), Amarok::TrueAudio );
        else if( file->ID3v1Tag() )
            tagHelper = new TagHelper( file->ID3v1Tag(), Amarok::TrueAudio );
    }
    else if( TagLib::WavPack::File *file = dynamic_cast< TagLib::WavPack::File * >( fileref.file() ) )
    {
        if( file->APETag( forceCreation ) )
            tagHelper = new APETagHelper( file->APETag(), Amarok::WavPack );
        else if( file->ID3v1Tag() )
            tagHelper = new TagHelper( file->ID3v1Tag(), Amarok::WavPack );
    }

    return tagHelper;
}
