#include "framelesswindow.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QGraphicsDropShadowEffect>
#include <QScreen>

#include "ui_framelesswindow.h"

FramelessWindow::FramelessWindow(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::FramelessWindow),
      m_bMousePressed(false),
      m_bDragTop(false),
      m_bDragLeft(false),
      m_bDragRight(false),
      m_bDragBottom(false) {

  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  // append minimize button flag in case of windows,
  // for correct windows native handling of minimize function
#if defined(Q_OS_WIN)
  setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint | Qt::WindowStaysOnTopHint);
#endif
  setAttribute(Qt::WA_NoSystemBackground, true);
  setAttribute(Qt::WA_TranslucentBackground);

  ui->setupUi(this);

  ui->minimizeButton->setVisible(false);
  ui->maximizeButton->setVisible(false);

  // shadow under window title text
  QGraphicsDropShadowEffect *textShadow = new QGraphicsDropShadowEffect;
  textShadow->setBlurRadius(4.0);
  textShadow->setColor(QColor(0, 0, 0));
  textShadow->setOffset(0.0);
  //ui->titleText->setGraphicsEffect(textShadow);

  // window shadow
  QGraphicsDropShadowEffect *windowShadow = new QGraphicsDropShadowEffect;
  windowShadow->setBlurRadius(9.0);
  windowShadow->setColor(palette().color(QPalette::Highlight));
  windowShadow->setOffset(0.0);
  //ui->windowFrame->setGraphicsEffect(windowShadow);

  QObject::connect(qApp, &QGuiApplication::applicationStateChanged, this,
                   &FramelessWindow::on_applicationStateChanged);
  setMouseTracking(true);

  // important to watch mouse move from all child widgets
  QApplication::instance()->installEventFilter(this);
}

FramelessWindow::~FramelessWindow()
{
    delete ui;
}

void FramelessWindow::on_restoreButton_clicked() {
  setWindowState(Qt::WindowNoState);
}

void FramelessWindow::on_maximizeButton_clicked() {
    if (windowState().testFlag(Qt::WindowNoState)) {
        this->setWindowState(Qt::WindowMaximized);
        this->showMaximized();
        styleWindow(true, false);
    } else if (windowState().testFlag(Qt::WindowFullScreen) || windowState().testFlag(Qt::WindowMaximized)) {
        on_restoreButton_clicked();
        return;
    }
}

void FramelessWindow::changeEvent(QEvent *event) {
  if (event->type() == QEvent::WindowStateChange) {
    if (windowState().testFlag(Qt::WindowNoState)) {
      styleWindow(true, true);
      event->ignore();
    } else if (windowState().testFlag(Qt::WindowMaximized)) {
      styleWindow(true, false);
      event->ignore();
    }
  }
  event->accept();
}

void FramelessWindow::setContent(QWidget *w) {
  ui->windowContent->layout()->addWidget(w);
}

void FramelessWindow::setWindowTitle(const QString &text) {
  ui->titleText->setText(text);
}

void FramelessWindow::setWindowIcon(const QIcon &ico) {
	Q_UNUSED(ico);
}

void FramelessWindow::styleWindow(bool bActive, bool bNoState) {
	Q_UNUSED(bActive);
	Q_UNUSED(bNoState);
	return;
}

void FramelessWindow::on_applicationStateChanged(Qt::ApplicationState state) {
    if (windowState().testFlag(Qt::WindowNoState)) {
        if (state == Qt::ApplicationActive) {
            styleWindow(true, true);
        } else {
            styleWindow(false, true);
        }
    } else if (windowState().testFlag(Qt::WindowFullScreen)) {
        if (state == Qt::ApplicationActive) {
            styleWindow(true, false);
        } else {
            styleWindow(false, false);
        }
    }
}

void FramelessWindow::on_minimizeButton_clicked() {
  setWindowState(Qt::WindowMinimized);
}

void FramelessWindow::on_closeButton_clicked()
{
    emit requestClose();
}

void FramelessWindow::on_windowTitlebar_doubleClicked()
{
    return;
    if (windowState().testFlag(Qt::WindowNoState)) {
        on_maximizeButton_clicked();
    } else if (windowState().testFlag(Qt::WindowFullScreen) || windowState().testFlag(Qt::WindowMaximized)) {
        on_restoreButton_clicked();
    }
}

void FramelessWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
  Q_UNUSED(event);
}

void FramelessWindow::checkBorderDragging(QMouseEvent *event)
{
    if (isMaximized()) {
        return;
    }

    QPoint globalMousePos = event->globalPos();
    if (m_bMousePressed) {
        QScreen *screen = QGuiApplication::primaryScreen();
        // available geometry excludes taskbar
        QRect availGeometry = screen->availableGeometry();
        int h = availGeometry.height();
        int w = availGeometry.width();
        QList<QScreen *> screenlist = screen->virtualSiblings();
        if (screenlist.contains(screen)) {
            QSize sz = QApplication::desktop()->size();
            h = sz.height();
            w = sz.width();
        }

        // top right corner
        if (m_bDragTop && m_bDragRight) {
            int diff = globalMousePos.x() - (m_StartGeometry.x() + m_StartGeometry.width());
            int neww = m_StartGeometry.width() + diff;
            diff = globalMousePos.y() - m_StartGeometry.y();
            int newy = m_StartGeometry.y() + diff;
            if (neww > 0 && newy > 0 && newy < h - 50 &&
                    m_StartGeometry.height() - diff > minimumHeight()) {
                QRect newg = m_StartGeometry;
                newg.setWidth(neww);
                newg.setX(m_StartGeometry.x());
                newg.setY(newy);
                setGeometry(newg);
            }
        }
        // top left corner
        else if (m_bDragTop && m_bDragLeft) {
            int diff = globalMousePos.y() - m_StartGeometry.y();
            int newy = m_StartGeometry.y() + diff;
            diff = globalMousePos.x() - m_StartGeometry.x();
            int newx = m_StartGeometry.x() + diff;
            if (newy > 0 && newx > 0 &&
                    m_StartGeometry.width() - diff > minimumWidth() &&
                    m_StartGeometry.height() - diff > minimumHeight()) {
                QRect newg = m_StartGeometry;
                newg.setY(newy);
                newg.setX(newx);
                setGeometry(newg);
            }
        }
        // bottom right corner
        else if (m_bDragBottom && m_bDragLeft) {
            int diff = globalMousePos.y() - (m_StartGeometry.y() + m_StartGeometry.height());
            int newh = m_StartGeometry.height() + diff;
            diff = globalMousePos.x() - m_StartGeometry.x();
            int newx = m_StartGeometry.x() + diff;
            if (newh > 0 && newx > 0 &&
                    m_StartGeometry.width() - diff > minimumWidth()) {
                QRect newg = m_StartGeometry;
                newg.setX(newx);
                newg.setHeight(newh);
                setGeometry(newg);
            }
        } else if (m_bDragTop) {
            int diff = globalMousePos.y() - m_StartGeometry.y();
            int newy = m_StartGeometry.y() + diff;
            if (newy > 0 && newy < h - 50 &&
                    m_StartGeometry.height() - diff > minimumHeight()) {
                QRect newg = m_StartGeometry;
                newg.setY(newy);
                setGeometry(newg);
            }
        } else if (m_bDragLeft) {
            int diff = globalMousePos.x() - m_StartGeometry.x();
            int newx = m_StartGeometry.x() + diff;
            if (newx > 0 && newx < w - 50 &&
                    m_StartGeometry.width() - diff > minimumWidth()) {
                QRect newg = m_StartGeometry;
                newg.setX(newx);
                setGeometry(newg);
            }
        } else if (m_bDragRight) {
            int diff = globalMousePos.x() - (m_StartGeometry.x() + m_StartGeometry.width());
            int neww = m_StartGeometry.width() + diff;
            if (neww > 0) {
                QRect newg = m_StartGeometry;
                newg.setWidth(neww);
                newg.setX(m_StartGeometry.x());
                setGeometry(newg);
            }
        } else if (m_bDragBottom) {
            int diff = globalMousePos.y() - (m_StartGeometry.y() + m_StartGeometry.height());
            int newh = m_StartGeometry.height() + diff;
            if (newh > 0) {
                QRect newg = m_StartGeometry;
                newg.setHeight(newh);
                newg.setY(m_StartGeometry.y());
                setGeometry(newg);
            }
        }
    } else {
        // no mouse pressed
        if (leftBorderHit(globalMousePos) && topBorderHit(globalMousePos)) {
            setCursor(Qt::SizeFDiagCursor);
        } else if (rightBorderHit(globalMousePos) && topBorderHit(globalMousePos)) {
            setCursor(Qt::SizeBDiagCursor);
        } else if (leftBorderHit(globalMousePos) &&
                   bottomBorderHit(globalMousePos)) {
            setCursor(Qt::SizeBDiagCursor);
        } else {
            if (topBorderHit(globalMousePos)) {
                setCursor(Qt::SizeVerCursor);
            } else if (leftBorderHit(globalMousePos)) {
                setCursor(Qt::SizeHorCursor);
            } else if (rightBorderHit(globalMousePos)) {
                setCursor(Qt::SizeHorCursor);
            } else if (bottomBorderHit(globalMousePos)) {
                setCursor(Qt::SizeVerCursor);
            } else {
                m_bDragTop = false;
                m_bDragLeft = false;
                m_bDragRight = false;
                m_bDragBottom = false;
                setCursor(Qt::ArrowCursor);
            }
        }
    }
}

// pos in global virtual desktop coordinates
bool FramelessWindow::leftBorderHit(const QPoint &pos) {
  const QRect &rect = this->geometry();
  if (pos.x() >= rect.x() && pos.x() <= rect.x() + CONST_DRAG_BORDER_SIZE) {
    return true;
  }
  return false;
}

bool FramelessWindow::rightBorderHit(const QPoint &pos) {
  const QRect &rect = this->geometry();
  int tmp = rect.x() + rect.width();
  if (pos.x() <= tmp && pos.x() >= (tmp - CONST_DRAG_BORDER_SIZE)) {
    return true;
  }
  return false;
}

bool FramelessWindow::topBorderHit(const QPoint &pos) {
  const QRect &rect = this->geometry();
  if (pos.y() >= rect.y() && pos.y() <= rect.y() + CONST_DRAG_BORDER_SIZE) {
    return true;
  }
  return false;
}

bool FramelessWindow::bottomBorderHit(const QPoint &pos) {
  const QRect &rect = this->geometry();
  int tmp = rect.y() + rect.height();
  if (pos.y() <= tmp && pos.y() >= (tmp - CONST_DRAG_BORDER_SIZE)) {
    return true;
  }
  return false;
}

void FramelessWindow::mousePressEvent(QMouseEvent *event) {
  if (isMaximized()) {
    return;
  }

  m_bMousePressed = true;
  m_StartGeometry = this->geometry();

  QPoint globalMousePos = mapToGlobal(QPoint(event->x(), event->y()));

  if (leftBorderHit(globalMousePos) && topBorderHit(globalMousePos)) {
    m_bDragTop = true;
    m_bDragLeft = true;
    setCursor(Qt::SizeFDiagCursor);
  } else if (rightBorderHit(globalMousePos) && topBorderHit(globalMousePos)) {
    m_bDragRight = true;
    m_bDragTop = true;
    setCursor(Qt::SizeBDiagCursor);
  } else if (leftBorderHit(globalMousePos) && bottomBorderHit(globalMousePos)) {
    m_bDragLeft = true;
    m_bDragBottom = true;
    setCursor(Qt::SizeBDiagCursor);
  } else {
    if (topBorderHit(globalMousePos)) {
      m_bDragTop = true;
      setCursor(Qt::SizeVerCursor);
    } else if (leftBorderHit(globalMousePos)) {
      m_bDragLeft = true;
      setCursor(Qt::SizeHorCursor);
    } else if (rightBorderHit(globalMousePos)) {
      m_bDragRight = true;
      setCursor(Qt::SizeHorCursor);
    } else if (bottomBorderHit(globalMousePos)) {
      m_bDragBottom = true;
      setCursor(Qt::SizeVerCursor);
    }
  }
}

void FramelessWindow::mouseReleaseEvent(QMouseEvent *event) {
  Q_UNUSED(event);
  if (isMaximized()) {
    return;
  }

  m_bMousePressed = false;
  bool bSwitchBackCursorNeeded =
      m_bDragTop || m_bDragLeft || m_bDragRight || m_bDragBottom;
  m_bDragTop = false;
  m_bDragLeft = false;
  m_bDragRight = false;
  m_bDragBottom = false;
  if (bSwitchBackCursorNeeded) {
    setCursor(Qt::ArrowCursor);
  }
}

bool FramelessWindow::eventFilter(QObject *obj, QEvent *event) {
  if (isMaximized()) {
    return QWidget::eventFilter(obj, event);
  }

  // check mouse move event when mouse is moved on any object
  if (event->type() == QEvent::MouseMove) {
    QMouseEvent *pMouse = dynamic_cast<QMouseEvent *>(event);
    if (pMouse) {
      checkBorderDragging(pMouse);
    }
  }
  // press is triggered only on frame window
  else if (event->type() == QEvent::MouseButtonPress && obj == this) {
    QMouseEvent *pMouse = dynamic_cast<QMouseEvent *>(event);
    if (pMouse) {
      mousePressEvent(pMouse);
    }
  } else if (event->type() == QEvent::MouseButtonRelease) {
    if (m_bMousePressed) {
      QMouseEvent *pMouse = dynamic_cast<QMouseEvent *>(event);
      if (pMouse) {
        mouseReleaseEvent(pMouse);
      }
    }
  }

  return QWidget::eventFilter(obj, event);
}
