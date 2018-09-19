/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez / xtingray                                          *
 *                                                                         *
 *   KTooN's versions:                                                     * 
 *                                                                         *
 *   2006:                                                                 *
 *    David Cuadrado                                                       *
 *    Jorge Cuadrado                                                       *
 *   2003:                                                                 *
 *    Fernado Roldan                                                       *
 *    Simena Dinas                                                         *
 *                                                                         *
 *   Copyright (C) 2010 Gustav Gonzalez - http://www.maefloresta.com       *
 *   License:                                                              *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "tmainwindow.h"

class T_GUI_EXPORT DefaultSettings : public TMainWindowAbstractSettings
{
    public:
        DefaultSettings(QObject *parent);
        ~DefaultSettings();
        void save(TMainWindow *window);
        void restore(TMainWindow *window);
};

DefaultSettings::DefaultSettings(QObject *parent) : TMainWindowAbstractSettings(parent)
{
}

DefaultSettings::~DefaultSettings()
{
}

void DefaultSettings::save(TMainWindow *window)
{
    #ifdef TUP_DEBUG
        QString msg = "TMainWindow::DefaultSettings::save() - Saving UI settings [ " + qApp->applicationName() + " ]";
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tWarning() << msg;
        #endif
    #endif

    QSettings settings(qApp->applicationName(), "ideality", this);

    QHash<Qt::ToolBarArea, TButtonBar *> buttonBars = window->buttonBars();
    QHash<TButtonBar *, QList<ToolView*> > toolViews = window->toolViews();

    foreach (TButtonBar *bar, buttonBars.values()) {
        foreach (ToolView *view, toolViews[bar]) {
            settings.beginGroup(view->objectName());
            settings.setValue("area", int(view->button()->area()));
            // settings.setValue("size", view->fixedSize());
            settings.setValue("style", view->button()->toolButtonStyle());
            // settings.setValue("sensibility", view->button()->isSensible());
            settings.setValue("visible", view->isVisible());
            settings.setValue("floating", view->isFloating());
            settings.setValue("position", view->pos());
            settings.endGroup();
        }
    }

    settings.beginGroup("MainWindow");
    settings.setValue("size", window->size());
    settings.setValue("maximized", window->isMaximized());
    settings.setValue("position", window->pos());
    settings.endGroup();
}

void DefaultSettings::restore(TMainWindow *window)
{
    #ifdef TUP_DEBUG
	QString msg = "TMainWindow::DefaultSettings::restore() - Restoring UI settings [ " + qApp->applicationName() + " ]";
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tWarning() << msg;
        #endif
    #endif

    QSettings settings(qApp->applicationName(), "ideality", this);

    QHash<Qt::ToolBarArea, TButtonBar *> buttonBars = window->buttonBars();
    QHash<TButtonBar *, QList<ToolView*> > toolViews = window->toolViews();

    QList<ToolView *> toHide;

    foreach (TButtonBar *bar, buttonBars.values()) {
        foreach (ToolView *view, toolViews[bar]) {
            settings.beginGroup(view->objectName());

            // Restore position
            Qt::DockWidgetArea area = Qt::DockWidgetArea(settings.value("area", 0).toInt());
            window->moveToolView(view, area);
            // view->setFixedSize(settings.value("size").toInt());

            view->button()->setToolButtonStyle(Qt::ToolButtonStyle(settings.value("style", 
                                               int(view->button()->toolButtonStyle())).toInt()));
            // view->button()->setSensible(settings.value("sensibility", view->button()->isSensible()).toBool());

            bool visible = settings.value("visible", false).toBool();
            if (visible && view->button()->isVisible()) {
                view->button()->setChecked(true);
                view->show();
            } else {
                toHide << view;
            }

            view->setFloating(settings.value("floating", false).toBool());

            if (view->isFloating())
                view->move(settings.value("position").toPoint());

            settings.endGroup();
        }
    }
	
    foreach (ToolView *view, toHide) {
        view->button()->setChecked(false);
        view->setVisible(false);
        view->close();
    }

    settings.beginGroup("MainWindow");
    window->resize(settings.value("size").toSize());
    bool maximized = settings.value("maximized", false).toBool();

    if (maximized)
        window->showMaximized();

    window->move(settings.value("position").toPoint());
    settings.endGroup();
}

// TMainWindow
TMainWindow::TMainWindow(QWidget *parent) : QMainWindow(parent), m_forRelayout(0), 
                                            m_currentPerspective(DefaultPerspective), m_autoRestore(false)
{
    setObjectName("TMainWindow");

    m_settings = new DefaultSettings(this);

    specialToolBar = new QToolBar(tr("Show Top Panel"), this);
    specialToolBar->setIconSize(QSize(9, 5));

    addToolBar(Qt::LeftToolBarArea, specialToolBar);
    addButtonBar(Qt::LeftToolBarArea);
    addButtonBar(Qt::RightToolBarArea);
    addButtonBar(Qt::TopToolBarArea);
    addButtonBar(Qt::BottomToolBarArea);

    setDockNestingEnabled(false);
}

TMainWindow::~TMainWindow()
{
}

void TMainWindow::addButtonBar(Qt::ToolBarArea area)
{
    TButtonBar *bar = new TButtonBar(area, this);
    addToolBar(area, bar);
    m_buttonBars.insert(area, bar);
    // bar->hide();
}

void TMainWindow::enableSpecialBar(bool flag)
{
    specialToolBar->setVisible(flag);
}

void TMainWindow::addSpecialButton(TAction *action)
{
    specialToolBar->addAction(action);
}

ToolView *TMainWindow::addToolView(QWidget *widget, Qt::DockWidgetArea area, int perspective, const QString &code, QKeySequence shortcut)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TMainWindow::addToolView()]";
        #else
            T_FUNCINFO << "- component: " << code;
        #endif
    #endif

    ToolView *toolView = new ToolView(widget->windowTitle(), widget->windowIcon(), code);

    // SQA: Make this option available from the Preferences dialog
    // toolView->setFeatures(!QDockWidget::DockWidgetMovable|!QDockWidget::DockWidgetFloatable);

    toolView->setShortcut(shortcut);
    toolView->setWidget(widget);
    toolView->setPerspective(perspective);
    toolView->button()->setArea(toToolBarArea(area));
    m_buttonBars[toToolBarArea(area)]->addButton(toolView->button());

    addDockWidget(area, toolView);
    m_toolViews[m_buttonBars[toToolBarArea(area)]] << toolView;

    /*
    // Show separators
    if (area == Qt::TopDockWidgetArea || area == Qt::BottomDockWidgetArea) 
        m_buttonBars[toToolBarArea(area)]->showSeparator(!m_buttonBars[Qt::LeftToolBarArea]->isEmpty());
    else if (area == Qt::LeftDockWidgetArea)
        m_buttonBars[Qt::TopToolBarArea]->showSeparator(m_buttonBars[Qt::TopToolBarArea]->isEmpty());

    if (toolView->isVisible())
        toolView->expandDock(false);
    */

    return toolView;
}

void TMainWindow::removeToolView(ToolView *view)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TMainWindow::removeToolView()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    bool findIt = false;

    foreach (TButtonBar *bar, m_buttonBars.values()) {
        QList<ToolView *> views = m_toolViews[bar];
        QList<ToolView *>::iterator it = views.begin();

        while (it != views.end()) {
            ToolView *toolView = *it;
            if (toolView == view) {
                views.erase(it);
                bar->removeButton(view->button());
                findIt = true;
                break;
            }
            ++it;
        }

        if (findIt) 
            break;
    }

    if (findIt)
        removeDockWidget(view);
}

void TMainWindow::enableToolViews(bool flag)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TMainWindow::enableToolViews()]";
        #else
            T_FUNCINFO << flag;
        #endif
    #endif

    foreach (TButtonBar *bar, m_buttonBars.values()) {
        QList<ToolView *> views = m_toolViews[bar];
        QList<ToolView *>::iterator it = views.begin();

        while (it != views.end()) {
            ToolView *view = *it;
            view->enableButton(flag);
            ++it;
        }
    }
}

void TMainWindow::moveToolView(ToolView *view, Qt::DockWidgetArea newPlace)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TMainWindow::moveToolView()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (toDockWidgetArea(view->button()->area()) == newPlace || newPlace == Qt::AllDockWidgetAreas || newPlace == 0)
        return;

    addDockWidget(newPlace, view);
    m_forRelayout = view;
}

void TMainWindow::addToPerspective(QWidget *widget, int perspective)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TMainWindow::addToPerspective()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (QToolBar *bar = dynamic_cast<QToolBar*>(widget)) {
        if (toolBarArea(bar) == 0)
            addToolBar(bar);
    }

    if (!m_managedWidgets.contains(widget)) {
        m_managedWidgets.insert(widget, perspective);

        if (!(perspective & m_currentPerspective)) 
            widget->hide();
    }
}

void TMainWindow::removeFromPerspective(QWidget *widget)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TMainWindow::removeFromPerspective()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    m_managedWidgets.remove(widget);
}

// Add action list to perspective
void TMainWindow::addToPerspective(const QList<QAction *> &actions, int perspective)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TMainWindow::addToPerspective()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    foreach (QAction *action, actions)
        addToPerspective(action, perspective);
}

// Add action to perspective
void TMainWindow::addToPerspective(QAction *action, int perspective)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TMainWindow::addToFromPerspective()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (!m_managedActions.contains(action)) {
        m_managedActions.insert(action, perspective);

        if (!(perspective & m_currentPerspective))
            action->setVisible(false);
    }
}

// Remove action from perspective
void TMainWindow::removeFromPerspective(QAction *action)
{
    m_managedActions.remove(action);
}

Qt::DockWidgetArea TMainWindow::toDockWidgetArea(Qt::ToolBarArea area)
{
    switch (area) {
        case Qt::LeftToolBarArea:
           {
             return Qt::LeftDockWidgetArea;
           }
           break;
        case Qt::RightToolBarArea:
           {
             return Qt::RightDockWidgetArea;
           }
           break;
        case Qt::TopToolBarArea:
           {
             return Qt::TopDockWidgetArea;
           }
           break;
        case Qt::BottomToolBarArea:
           {
             return Qt::BottomDockWidgetArea;
           }
           break;
        default: 
           {
              #ifdef TUP_DEBUG
                   QString msg = "TMainWindow::toDockWidgetArea() - Floating -> " + QString::number(area);
                   #ifdef Q_OS_WIN
                       qWarning() << msg;
                   #else
                       tWarning() << msg;
                   #endif
               #endif
           }
           break;
    }

    return Qt::LeftDockWidgetArea;
}

Qt::ToolBarArea TMainWindow::toToolBarArea(Qt::DockWidgetArea area)
{
    switch (area) {
        case Qt::LeftDockWidgetArea:
           {
             return Qt::LeftToolBarArea;
           }
           break;
        case Qt::RightDockWidgetArea:
           {
             return Qt::RightToolBarArea;
           }
           break;
        case Qt::TopDockWidgetArea:
           {
             return Qt::TopToolBarArea;
           }
           break;
        case Qt::BottomDockWidgetArea:
           {
             return Qt::BottomToolBarArea;
           }
           break;
        default: 
           {
             #ifdef TUP_DEBUG
                 QString msg = "TMainWindow::toToolBarArea() - Floating -> " + QString::number(area);
                 #ifdef Q_OS_WIN
                     qWarning() << msg;
                 #else
                     tWarning() << msg;
                 #endif
             #endif
           }
           break;
    }

    return Qt::LeftToolBarArea;
}

void TMainWindow::setCurrentPerspective(int workspace)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TMainWindow::setCurrentPerspective()]";
        #else
            T_FUNCINFO << workspace;
        #endif
    #endif

    if (m_currentPerspective == workspace)
        return;

    if (workspace != 1)
        specialToolBar->setVisible(false);
    else
        specialToolBar->setVisible(true);

    typedef QList<ToolView *> Views;
    QList<Views > viewsList = m_toolViews.values();

    /*
    setUpdatesEnabled(false);
    if (centralWidget())
        centralWidget()->setUpdatesEnabled(false);
    */

    QHash<TButtonBar *, int> hideButtonCount;
    foreach (Views views, viewsList) {
        foreach (ToolView *view, views) {
            TButtonBar *bar = m_buttonBars[view->button()->area()];
            // bar->setUpdatesEnabled(false);
            // view->setUpdatesEnabled(false);

            if (view->perspective() & workspace) {
                bar->enable(view->button());
                if (view->isExpanded()) {
                    view->blockSignals(true);
                    view->show(); 
                    view->blockSignals(false);
                }
            } else {
                bar->disable(view->button());
                if (view->isExpanded()) {
                    view->blockSignals(true);
                    view->close();
                    view->blockSignals(false);
                }
                hideButtonCount[bar]++;
            }

            if (bar->isEmpty() && bar->isVisible()) {
                bar->hide();
            } else {
                if (!bar->isVisible())
                    bar->show();
            }

            // view->setUpdatesEnabled(true);
            // bar->setUpdatesEnabled(true);
        }
    }

    QHashIterator<TButtonBar *, int> barIt(hideButtonCount);
    // This loop hides the bars with no buttons
    while (barIt.hasNext()) {
        barIt.next();
        if (barIt.key()->count() == barIt.value())
            barIt.key()->hide();
    }

    /*
    if (centralWidget())
        centralWidget()->setUpdatesEnabled(true);
    setUpdatesEnabled(true);
    */

    m_currentPerspective = workspace;
    emit perspectiveChanged(m_currentPerspective);
}

int TMainWindow::currentPerspective() const
{
    return m_currentPerspective;
}

// if autoRestore is true, the widgets will be loaded when main window is showed (position and properties)
void TMainWindow::setAutoRestore(bool autoRestore)
{
    m_autoRestore = autoRestore;
}

bool TMainWindow::autoRestore() const
{
    return m_autoRestore;
}

QMenu *TMainWindow::createPopupMenu()
{
    return 0;
}

void TMainWindow::setSettingsHandler(TMainWindowAbstractSettings *settings)
{
    delete m_settings;

    m_settings = settings;
    m_settings->setParent(this);
}

void TMainWindow::closeEvent(QCloseEvent *e)
{
    saveGUI();
    QMainWindow::closeEvent(e);
}

void TMainWindow::showEvent(QShowEvent *e)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TMainWindow::showEvent()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    QMainWindow::showEvent(e);

    if (!m_autoRestore) {
        m_autoRestore = true;
        restoreGUI();
        int cwsp = m_currentPerspective;
        m_currentPerspective -= 1;
        setCurrentPerspective(cwsp);
    }
}

void TMainWindow::saveGUI()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TMainWindow::saveGUI()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    m_settings->save(this);
}

void TMainWindow::restoreGUI()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TMainWindow::restoreGUI()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    setUpdatesEnabled(false);
    m_settings->restore(this);
    setUpdatesEnabled(true);
}

QHash<Qt::ToolBarArea, TButtonBar *> TMainWindow::buttonBars() const
{
    return m_buttonBars;
}

QHash<TButtonBar *, QList<ToolView*> > TMainWindow::toolViews() const
{
    return m_toolViews;
}
