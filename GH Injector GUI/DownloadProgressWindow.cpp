/*
 * Author:       Broihon
 * Copyright:    Guided Hacking� � 2012-2023 Guided Hacking LLC
 */

#include "pch.h"

#include "DownloadProgressWindow.h"

DownloadProgressWindow::DownloadProgressWindow(const QString &title, const std::vector<QString> &labels, const QString &status, int width,
                                               QWidget *parent) :
    QDialog(parent)
{
    auto *main_layout = new (std::nothrow) QVBoxLayout();
    if (main_layout == Q_NULLPTR)
    {
        THROW("Failed to create main layout for download window.");
    }

    for (const auto &i : labels)
    {
        auto *layout = new (std::nothrow) QHBoxLayout();
        if (layout == Q_NULLPTR)
        {
            THROW("Failed to create layout for download window.");
        }

        auto *bar = new (std::nothrow) QProgressBar();
        if (bar == Q_NULLPTR)
        {
            THROW("Failed to create progress bar for download window.");
        }

        bar->setMinimum(0);
        bar->setMaximum(100);
        bar->setFixedWidth(width);

        if (i.length() > 0)
        {
            auto *txt = new (std::nothrow) QLabel();
            if (txt == Q_NULLPTR)
            {
                THROW("Failed to create label for download window.");
            }

            txt->setText(i);
            layout->addWidget(txt);
        }

        layout->addWidget(bar);

        main_layout->addLayout(layout);

        m_ProgressBarList.push_back(bar);

        m_NewProgress.push_back(0.0f);
    }

    if (status.length() > 0)
    {
        auto *layout = new (std::nothrow) QHBoxLayout();
        if (layout == Q_NULLPTR)
        {
            THROW("Failed to create layout for download window.");
        }

        m_pStatus = new (std::nothrow) QLabel();
        if (m_pStatus == Q_NULLPTR)
        {
            THROW("Failed to create label for download window.");
        }

        m_pStatus->setText(status);

        layout->addWidget(m_pStatus);
        main_layout->addLayout(layout);
    }

    setLayout(main_layout);

    setFixedSize(QSize(width, 20 + (int)labels.size() * 40));
    setWindowIcon(QIcon(":/GuiMain/gh_resource/GH Icon.ico"));
    setWindowTitle(title);

    connect(&m_TmrCallback, SIGNAL(timeout()), this, SLOT(on_timer_callback()));

    m_TmrCallback.setInterval(100);

    installEventFilter(this);
    installEventFilter(this);
}

// this is the second worst code in the history of mankind but I really don't give a fuck
// because multithreading with QDialogs is the the worst thing in the history of mankind

void DownloadProgressWindow::on_timer_callback()
{
    if (m_pCallback && m_bRunning)
    {
        m_pCallback(this, m_pCustomArg);
    }
}

bool DownloadProgressWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (!m_bTimerRunning)
    {
        m_bTimerRunning = true;

        m_TmrCallback.start();
    }

    if (event->type() == QEvent::Close && obj == this)
    {
        m_TmrCallback.stop();

        m_bTimerRunning = false;
    }

    if (obj == this)
    {
        if (event->type() == QEvent::UpdateRequest)
        {
            if (m_bUpdateStatus)
            {
                m_bUpdateStatus = false;
                m_pStatus->setText(m_NewStatus);
            }

            if (m_bUpdateProgress)
            {
                m_bUpdateProgress = false;
                for (UINT i = 0; i < m_ProgressBarList.size(); ++i)
                {
                    m_ProgressBarList[i]->setValue(static_cast<int>(m_NewProgress[i]));
                }
            }
        }

        if (m_bUpdateDone)
        {
            m_bUpdateDone = false;
            done(m_NewDone);
        }
    }

    g_Console->update_external();

    return QObject::eventFilter(obj, event);
}

void DownloadProgressWindow::SetProgress(UINT index, float value)
{
    if (index < m_ProgressBarList.size())
    {
        int progress = 0;

        if (value >= 1.0f)
        {
            progress = 100;
        }
        else if (value <= 0.0f)
        {
            progress = 0;
        }
        else
        {
            value = value * 100.0f + 0.5f;
            progress = static_cast<int>(value);
        }

        m_NewProgress[index] = static_cast<float>(progress);
        m_bUpdateProgress = true;
        this->update();
    }
}

void DownloadProgressWindow::SetStatus(QString status)
{
    m_NewStatus = status;
    m_bUpdateStatus = true;
    this->update();
}

void DownloadProgressWindow::SetDone(int code)
{
    m_NewDone = code;
    m_bUpdateDone = true;
    this->update();
}

void DownloadProgressWindow::SetCallbackFrequency(int frequency)
{
    m_TmrCallback.setInterval(1000 / frequency);
}

void DownloadProgressWindow::SetCallbackArg(void *argument)
{
    m_pCustomArg = argument;
}

void DownloadProgressWindow::SetCallback(const decltype(m_pCallback) &callback)
{
    m_pCallback = callback;
}

void DownloadProgressWindow::SetCloseValue(int value)
{
    m_CustomCloseValue = value;
}

int DownloadProgressWindow::Execute()
{
    move(0, 0);
    setWindowModality(Qt::WindowModality::ApplicationModal);

    m_bRunning = true;
    auto result = exec();
    m_bRunning = false;

    move(0, 0);

    return result;
}

QString DownloadProgressWindow::GetStatus() const
{
    return m_pStatus->text();
}
