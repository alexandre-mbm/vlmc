/*****************************************************************************
 * ClipPreviewWidget: Preview widget
 *****************************************************************************
 * Copyright (C) 2008-2009 the VLMC team
 *
 * Authors: Geoffroy Lacarrière <geoffroylaca@gmail.com>
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

#include <QtDebug>

#include "ClipPreviewWidget.h"

ClipPreviewWidget::ClipPreviewWidget( QWidget* renderWidget ) :
    GenericPreviewWidget( renderWidget ),
    m_clipLoaded( false ), m_videoStopped( true ), m_vlcMedia( NULL )
{
}

ClipPreviewWidget::~ClipPreviewWidget()
{
}

void        ClipPreviewWidget::startPreview( Media* media )
{
    if ( m_vlcMedia != NULL )
        delete m_vlcMedia;
    m_vlcMedia = new LibVLCpp::Media( media->getVLCMedia() );

    m_mediaPlayer->setMedia( m_vlcMedia );

    connect( m_mediaPlayer,     SIGNAL( stopped() ),            this,   SLOT( __videoPaused() ) );
    connect( m_mediaPlayer,     SIGNAL( paused() ),             this,   SLOT( __videoPaused() ) );
    connect( m_mediaPlayer,     SIGNAL( playing() ),            this,   SLOT( __videoPlaying() ) );
    connect( m_mediaPlayer,     SIGNAL( positionChanged() ),    this,   SLOT( __positionChanged() ) );
    connect( m_mediaPlayer,     SIGNAL( endReached() ),         this,   SLOT( __endReached() ) );

    m_mediaPlayer->play();
    m_clipLoaded = true;
    m_videoStopped = false;
}

void        ClipPreviewWidget::setPosition( float newPos )
{
    if ( m_clipLoaded == false || m_videoStopped == true )
        return ;
    m_mediaPlayer->setPosition( newPos );
}

void        ClipPreviewWidget::stop()
{
    if ( m_clipLoaded == true && m_videoStopped == false )
    {
        m_videoStopped = true;
        m_mediaPlayer->stop();
    }
}

void        ClipPreviewWidget::togglePlayPause( bool forcePause )
{
    if ( m_clipLoaded == false)
        return ;
    if ( m_videoStopped == true )
        m_videoStopped = false;

    if ( m_mediaPlayer->isPlaying() )
        m_mediaPlayer->pause();
    else if ( forcePause == false )
        m_mediaPlayer->play();
}

/////////////////////////////////////////////////////////////////////
/////SLOTS :
/////////////////////////////////////////////////////////////////////
void        ClipPreviewWidget::__videoPaused()
{
    emit paused();
}

void        ClipPreviewWidget::__videoPlaying()
{
    emit playing();
}

void        ClipPreviewWidget::__positionChanged()
{
    if ( m_clipLoaded == false)
        return ;
    emit positionChanged( m_mediaPlayer->getPosition() );
}

void        ClipPreviewWidget::__endReached()
{
    m_mediaPlayer->stop();
    m_videoStopped = true;
    emit endReached();
}
