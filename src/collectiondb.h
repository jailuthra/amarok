// (c) 2004 Mark Kretschmann <markey@web.de>
// (c) 2004 Christian Muehlhaeuser <chris@chris.de>
// See COPYING file for licensing information.

#ifndef AMAROK_COLLECTIONDB_H
#define AMAROK_COLLECTIONDB_H

#include <qobject.h>         //baseclass
#include <qstringlist.h>     //stack allocated

class sqlite;
class ThreadWeaver;

class CollectionDB : public QObject
{
    Q_OBJECT
    
    public:
        CollectionDB();
        ~CollectionDB();

        QString albumSongCount( const QString artist_id, const QString album_id );
        void addImageToPath( const QString path, const QString image, bool temporary );
        QString getImageForAlbum( const QString artist_id, const QString album_id, const QString defaultImage );
        void incSongCounter( const QString url );
        void updateDirStats( QString path, const long datetime );
        void removeSongsInDir( QString path );
        bool isDirInCollection( QString path );
        void removeDirFromCollection( QString path );

        /**
         * Executes an SQL statement on the already opened database
         * @param statement SQL program to execute. Only one SQL statement is allowed.
         * @retval values   will contain the queried data, set to NULL if not used
         * @retval names    will contain all column names, set to NULL if not used
         * @return          true if successful
         */
        bool execSql( const QString& statement, QStringList* const values = 0, QStringList* const names = 0, const bool debug = false );

        /**
         * Returns the rowid of the most recently inserted row
         * @return          int rowid
         */
        int sqlInsertID();
        QString escapeString( QString string );

        uint getValueID( QString name, QString value, bool autocreate = true, bool useTempTables = false );
        void createTables( const bool temporary = false );
        void dropTables( const bool temporary = false );
        void moveTempTables();

        void purgeDirCache();
        void scanModifiedDirs( bool recursively );
        void scan( const QStringList& folders, bool recursively );
      
        void retrieveFirstLevel( QString category, QString filter, QStringList* const values, QStringList* const names );
        void retrieveSecondLevel( QString itemText, QString category1, QString category2, QString filter, QStringList* const values, QStringList* const names );
        void retrieveThirdLevel( QString itemText1, QString itemText2, QString category1, QString category2, QString filter, QStringList* const values, QStringList* const names );

        void retrieveFirstLevelURLs( QString itemText, QString category, QString filter, QStringList* const values, QStringList* const names );
        void retrieveSecondLevelURLs( QString itemText1, QString itemText2, QString category1, QString category2, QString filter, QStringList* const values, QStringList* const names );

    signals:
        void scanDone();

    private slots:
        void dirDirty( const QString& path );

    private:
        void customEvent( QCustomEvent* );

        sqlite* m_db;
        ThreadWeaver* m_weaver;
        bool m_monitor;
};


#endif /* AMAROK_COLLECTIONDB_H */
