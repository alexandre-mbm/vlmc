/*****************************************************************************
 * ProjectManager.cpp: Manager the project loading and saving.
 *****************************************************************************
 * Copyright (C) 2008-2009 the VLMC team
 *
 * Authors: Hugo Beauzee-Luyssen <hugo@vlmc.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#include <QFileDialog>
#include <QtDebug>

#include "ProjectManager.h"
#include "Library.h"
#include "MainWorkflow.h"
#include "SettingsManager.h"

ProjectManager::ProjectManager() : m_projectFile( NULL ), m_needSave( false )
{

}

ProjectManager::~ProjectManager()
{
    if ( m_projectFile != NULL )
        delete m_projectFile;
}

bool    ProjectManager::needSave() const
{
    return m_needSave;
}

void    ProjectManager::cleanChanged( bool val )
{
    m_needSave = !val;
    if ( m_projectFile != NULL )
    {
        QFileInfo   fInfo( *m_projectFile );
        emit projectChanged( fInfo.fileName(), val );
    }
    else
        emit projectChanged( tr( "<Unsaved project>" ), val );
}

void    ProjectManager::loadTimeline()
{
    QDomElement     root = m_domDocument->documentElement();
    QFileInfo       fInfo( *m_projectFile );

    MainWorkflow::getInstance()->loadProject( root.firstChildElement( "timeline" ) );
    emit projectChanged( fInfo.fileName(), true );
}

void    ProjectManager::loadProject( const QString& fileName )
{
    if ( fileName.length() == 0 )
        return;

    if ( !m_projectFile )
        delete m_projectFile;
    m_projectFile = new QFile( fileName );

    m_domDocument = new QDomDocument;
    m_projectFile->open( QFile::ReadOnly );
    m_domDocument->setContent( m_projectFile );
    m_projectFile->close();

    QDomElement     root = m_domDocument->documentElement();

    connect( Library::getInstance(), SIGNAL( projectLoaded() ), this, SLOT( loadTimeline() ) );
    Library::getInstance()->loadProject( root.firstChildElement( "medias" ) );
    SettingsManager::getInstance()->loadSettings( "project", root.firstChildElement( "project" ) );
}

QString  ProjectManager::loadProjectFile()
{
    QString fileName =
            QFileDialog::getOpenFileName( NULL, "Enter the output file name",
                                          QString(), "VLMC project file(*.vlmc)" );
    return fileName;
}

bool    ProjectManager::checkProjectOpen( bool saveAs )
{
    if ( m_projectFile == NULL || saveAs == true )
    {
        QString outputFileName =
            QFileDialog::getSaveFileName( NULL, "Enter the output file name",
                                          QString(), "VLMC project file(*.vlmc)" );
        if ( outputFileName.length() == 0 )
            return false;
        if ( m_projectFile != NULL )
            delete m_projectFile;
        if ( outputFileName.endsWith( ".vlmc" ) == false )
            outputFileName += ".vlmc";
        m_projectFile = new QFile( outputFileName );
    }
    return true;
}

void    ProjectManager::saveProject( bool saveAs /*= true*/ )
{
    if ( checkProjectOpen( saveAs ) == false )
        return ;
    QDomImplementation    implem = QDomDocument().implementation();
    //FIXME: Think about naming the project...
    QString name = "VLMCProject";
    QString publicId = "-//XADECK//DTD Stone 1.0 //EN";
    QString systemId = "http://www-imagis.imag.fr/DTD/stone1.dtd";
    QDomDocument doc(implem.createDocumentType( name, publicId, systemId ) );

    QDomElement rootNode = doc.createElement( "vlmc" );

    Library::getInstance()->saveProject( doc, rootNode );
    MainWorkflow::getInstance()->saveProject( doc, rootNode );
    SettingsManager::getInstance()->saveSettings( "project", doc, rootNode );
    SettingsManager::getInstance()->saveSettings( "keyboard_shortcut", doc, rootNode );

    doc.appendChild( rootNode );

    m_projectFile->open( QFile::WriteOnly );
    m_projectFile->write( doc.toString().toAscii() );
    m_projectFile->close();
    if ( saveAs == true )
    {
        QFileInfo   fInfo( *m_projectFile );
        emit projectChanged( fInfo.fileName(), true );
    }
    emit projectSaved();
}
