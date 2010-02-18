/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of duihome.
**
** If you have questions regarding the use of this file, please contact
** Nokia at directui@nokia.com.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/
#ifndef SWITCHERBUTTONGLESVIEW_H
#define SWITCHERBUTTONGLESVIEW_H

#include <DuiGLES2Renderer>
#include "switcherbuttonview.h"

/*!
 * \class SwitcherButtonGLESView
 *
 * \brief SwitcherButtonGLESView implements a switcher style view with OpenGL ES rendering
 */
class SwitcherButtonGLESView : public SwitcherButtonView
{
    Q_OBJECT
    DUI_VIEW(SwitcherButtonModel, SwitcherButtonStyle)

public:
    /*!
     * \brief Constructs a new SwitcherButtonGLESView.
     *
     * \param controller The button's controller
     */
    SwitcherButtonGLESView(SwitcherButton *controller);

    /*!
     * \brief Destroys the SwitcherButtonGLESView.
     */
    virtual ~SwitcherButtonGLESView();

private slots:
    //! \reimp
    virtual void damageEvent(Qt::HANDLE &damage, short &x, short &y, unsigned short &width, unsigned short &height);
    virtual void windowVisibilityChanged(Window window);
    //! \reimp_end

private:
    //! \reimp
    virtual void updateThumbnail();
    virtual void backendSpecificDrawBackground(QPainter *painter, const QStyleOptionGraphicsItem *option) const;
    //! \reimp_end

    //! Window texture ID
    quint32 windowTextureID;

    Q_DISABLE_COPY(SwitcherButtonGLESView);
};

#endif
