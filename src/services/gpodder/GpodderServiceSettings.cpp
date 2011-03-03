/****************************************************************************************
 * Copyright (c) 2007 Shane King <kde@dontletsstart.com>                                *
 * Copyright (c) 2010 Stefan Derkits <stefan@derkits.at>                                *
 * Copyright (c) 2010 Christian Wagner <christian.wagner86@gmx.at>                      *
 * Copyright (c) 2010 Felix Winter <ixos01@gmail.com>                                   *
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

#define DEBUG_PREFIX "GpodderServiceSettings"

#include "GpodderServiceSettings.h"

#include "core/support/Amarok.h"
#include "core/support/Debug.h"
#include "NetworkAccessManagerProxy.h"
#include "ui_GpodderConfigWidget.h"

#include <mygpo-qt/ApiRequest.h>

//#include <QCryptographicHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVBoxLayout>
#include <QRegExpValidator>

#include <KMessageBox>
#include <KPluginFactory>
#include <KLocale>


K_PLUGIN_FACTORY( GpodderServiceSettingsFactory, registerPlugin<GpodderServiceSettings>(); )
K_EXPORT_PLUGIN( GpodderServiceSettingsFactory( "kcm_amarok_gpodder" ) )

/*QString md5( const QByteArray& src )
{
    QByteArray const digest = QCryptographicHash::hash( src, QCryptographicHash::Md5 );
    return QString::fromLatin1( digest.toHex() ).rightJustified( 32, '0' );
}*/


GpodderServiceSettings::GpodderServiceSettings( QWidget *parent, const QVariantList &args )
        : KCModule( GpodderServiceSettingsFactory::componentData(), parent, args ), m_enableProvider( false ), m_createDevice( 0 )
{
    debug() << "Creating gpodder.net config object";

    QVBoxLayout* l = new QVBoxLayout( this );
    QWidget *w = new QWidget;
    m_configDialog = new Ui::GpodderConfigWidget;
    m_configDialog->setupUi( w );
    l->addWidget( w );
    m_configDialog->kcfg_SynchronisePodcasts->setVisible( false );

    connect( m_configDialog->kcfg_GpodderUsername, SIGNAL( textChanged( const QString & ) ), this, SLOT( settingsChanged() ) );
    connect( m_configDialog->kcfg_GpodderPassword, SIGNAL( textChanged( const QString & ) ), this, SLOT( settingsChanged() ) );
    connect( m_configDialog->testLogin, SIGNAL( clicked() ), this, SLOT( testLogin() ) );

    load();
}


GpodderServiceSettings::~GpodderServiceSettings()
{
    delete m_createDevice;
}


void GpodderServiceSettings::save()
{
    m_config.setUsername( m_configDialog->kcfg_GpodderUsername->text() );
    m_config.setPassword( m_configDialog->kcfg_GpodderPassword->text() );
    m_config.setSynchronise( m_configDialog->kcfg_SynchronisePodcasts->isChecked() );
    m_config.setEnableProvider( m_enableProvider );
    m_config.save();

    KCModule::save();
}

void GpodderServiceSettings::testLogin()
{
    DEBUG_BLOCK

    m_configDialog->testLogin->setEnabled( false );
    m_configDialog->testLogin->setText( i18n( "Testing..." ) );

    mygpo::ApiRequest api(m_configDialog->kcfg_GpodderUsername->text(),m_configDialog->kcfg_GpodderPassword->text(), The::networkAccessManager() );
    m_devices = api.listDevices( m_configDialog->kcfg_GpodderUsername->text() );

    connect( m_devices.data(), SIGNAL( finished() ), SLOT( finished() ) );
    connect( m_devices.data(), SIGNAL( requestError(QNetworkReply::NetworkError)), SLOT( onError( QNetworkReply::NetworkError ) ) );
    connect( m_devices.data(), SIGNAL( parseError()), SLOT( onParseError()) );
}

void GpodderServiceSettings::finished()
{
    DEBUG_BLOCK

    debug() << "Authentication worked, got List of Devices, searching for Amarok Device";
    m_configDialog->testLogin->setText( i18nc( "The operation completed as expected", "Success" ) );
    m_configDialog->testLogin->setEnabled( false );
    bool deviceExists = false;
    QList<mygpo::DevicePtr> ptrList = m_devices->devicesList();
    mygpo::DevicePtr devPtr;
    foreach (devPtr, ptrList) {
        if (devPtr->id().compare("amarok")==0)
        {
            deviceExists = true;
            break;
        }
    }
    if (!deviceExists)
    {
        mygpo::ApiRequest api(m_configDialog->kcfg_GpodderUsername->text(),m_configDialog->kcfg_GpodderPassword->text(), The::networkAccessManager() );
        m_createDevice = api.renameDevice(m_configDialog->kcfg_GpodderUsername->text(),QLatin1String("amarok"),QLatin1String("Device for gpodder.net in Amarok"),mygpo::Device::OTHER);
        connect( m_createDevice, SIGNAL( finished() ), SLOT( deviceCreationFinished()) );
        connect( m_createDevice, SIGNAL( error(QNetworkReply::NetworkError)), SLOT( deviceCreationError(QNetworkReply::NetworkError)) );
    }
    else {
        debug() << "amarok device was found, everything looks perfect";
        m_enableProvider = true;
        //m_configDialog->kcfg_SynchronisePodcasts->setEnabled( true );
    }
}

void GpodderServiceSettings::onError( QNetworkReply::NetworkError code )
{
    DEBUG_BLOCK

    debug() << code;

    if ( code == QNetworkReply::NoError )
        debug() << "No Error was found, but onError was called - should not happen";
    else if ( code == QNetworkReply::AuthenticationRequiredError )
    {
        debug() << "AuthenticationFailed";
        KMessageBox::error( this, i18n( "Either the username or the password is incorrect, please correct and try again" ), i18n( "Failed" ) );
        m_configDialog->testLogin->setText( i18n( "Test Login" ) );
        m_configDialog->testLogin->setEnabled( true );
        return;
    } else {
        KMessageBox::error( this, i18n( "Unable to connect to godder.net service or other Error occured." ), i18n( "Failed" ) );
        m_configDialog->testLogin->setText( i18n( "Test Login" ) );
        m_configDialog->testLogin->setEnabled( true );
        //m_configDialog->kcfg_SynchronisePodcasts->setCheckState( Qt::Unchecked );
        //m_configDialog->kcfg_SynchronisePodcasts->setEnabled(false);
    }


}

void GpodderServiceSettings::onParseError()
{
    debug() << "Couldn't parse DeviceList, should not happen if gpodder.net is working correctly";
    //m_configDialog->kcfg_SynchronisePodcasts->setCheckState( Qt::Unchecked );
    //m_configDialog->kcfg_SynchronisePodcasts->setEnabled(false);
    m_configDialog->testLogin->setEnabled( true );
    KMessageBox::error( this, i18n( "Error parsing the Reply, check if gpodder.net is working correctly and report a bug" ), i18n( "Failed" ) );
}

void GpodderServiceSettings::deviceCreationFinished()
{
    debug() << "Creation of Amarok Device finished";
    m_enableProvider = true;
    //m_configDialog->kcfg_SynchronisePodcasts->setEnabled(true);
}

void GpodderServiceSettings::deviceCreationError(QNetworkReply::NetworkError code)
{
    debug() << "Error creating Amarok Device";
    debug() << code;
    //m_configDialog->kcfg_SynchronisePodcasts->setCheckState( Qt::Unchecked );
    //m_configDialog->kcfg_SynchronisePodcasts->setEnabled(false);
    m_configDialog->testLogin->setEnabled( true );
}

void GpodderServiceSettings::load()
{
    m_config.load();
    m_configDialog->kcfg_GpodderUsername->setText( m_config.username() );
    m_configDialog->kcfg_GpodderPassword->setText( m_config.password() );
    //m_configDialog->kcfg_SynchronisePodcasts->setChecked( m_config.synchronise() );
    m_enableProvider = m_config.enableProvider();
    //if ( m_enableProvider )
    //    m_configDialog->kcfg_SynchronisePodcasts->setEnabled( true );

    KCModule::load();
}

void GpodderServiceSettings::defaults()
{
    m_config.reset();
    m_enableProvider = false;
    // By default this checkboxes is:
    //m_configDialog->kcfg_SynchronisePodcasts->setChecked( false );
}


void GpodderServiceSettings::settingsChanged()
{

    m_configDialog->testLogin->setText( i18n( "&Test Login" ) );
    m_configDialog->testLogin->setEnabled( true );
    //m_configDialog->kcfg_SynchronisePodcasts->setCheckState( Qt::Unchecked );
    //m_configDialog->kcfg_SynchronisePodcasts->setEnabled( false );
    m_enableProvider = false;
    emit changed( true );
}
