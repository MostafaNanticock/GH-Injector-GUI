/*
 * Author:       Broihon
 * Copyright:    Guided Hacking™ © 2012-2023 Guided Hacking LLC
 */

#include "pch.h"

#include "DebugConsole.h"

#include <QApplication>
#include <QFontDataBase>

#include <thread>

DebugConsole::DebugConsole(QWidget *parent) : QWidget(parent)
{
    QSizePolicy policy;
    policy.setHorizontalPolicy(QSizePolicy::Policy::MinimumExpanding);
    policy.setVerticalPolicy(QSizePolicy::Policy::MinimumExpanding);

    m_Content = new (std::nothrow) QListWidget();
    if (m_Content == Q_NULLPTR)
    {
        THROW("Failed to create content list widget for debug console.");
    }

    m_Content->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
    m_Content->setSizePolicy(policy);
    m_Content->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    m_Content->installEventFilter(this);

    m_Layout = new (std::nothrow) QGridLayout();
    if (m_Layout == Q_NULLPTR)
    {
        THROW("Failed to create content layout for debug console.");
    }

    QMargins m = {0, 0, 0, 0};
    this->setSizePolicy(policy);
    this->setContentsMargins(m);
    this->setLayout(m_Layout);
    this->layout()->addWidget(m_Content);
    this->installEventFilter(this);

    setWindowTitle("Debug data");
    setWindowIcon(QIcon(":/GuiMain/gh_resource/GH Icon.ico"));
    resize(QSize(150, 350));
}

void DebugConsole::open()
{
    show();
}

void DebugConsole::close()
{
    hide();
}

void DebugConsole::move(const QPoint &p)
{
    QWidget::move(p);
}

void DebugConsole::set_size(const QSize &s)
{
    resize(s);
}

int DebugConsole::print(const char *format, ...)
{
    int result = 0;
    int size = 1024;
    char *buffer = new (std::nothrow) char[size]();

    if (buffer == nullptr)
    {
        return -1;
    }

    auto old = _set_thread_local_invalid_parameter_handler(DebugConsole::ImTheTrashMan);

    do
    {
        va_list args;
        va_start(args, format);

        int err = 0;
        result = vsprintf_s(buffer, size, format, args);

        if (result <= 0)
        {
            err = errno;
        }

        va_end(args);

        if (result < 0 && err == ERANGE)
        {
            delete[] buffer;

            size += 1024;
            buffer = new (std::nothrow) char[size]();

            if (buffer == nullptr)
            {
                break;
            }
        }
        else if (result < 0)
        {
            break;
        }
    } while (result < 0);

    _set_thread_local_invalid_parameter_handler(old);

    if (result > 0)
    {
        print_raw(buffer);

        m_Content->scrollToBottom();
    }

    if (buffer)
    {
        delete[] buffer;
    }

    return result;
}

void DebugConsole::print_raw(const char *szText)
{
    if (!szText)
    {
        return;
    }

    char *copy = nullptr;
    bool use_copy = false;

    size_t len = lstrlenA(szText);

    if (len > 0)
    {
        if (szText[len - 1] == '\n')
        {
            use_copy = true;

            copy = new (std::nothrow) char[len]();
            if (copy == nullptr)
            {
                return;
            }

            memcpy(copy, szText, len - 1);
        }
    }
    else
    {
        return;
    }

    if (use_copy)
    {
        szText = copy;
    }

#ifdef DEBUG_CONSOLE_TO_CMD
    printf("CONSOLE: %s\n", szText);
#endif

    QListWidgetItem *new_item = new (std::nothrow) QListWidgetItem();
    if (new_item == nullptr)
    {
        if (use_copy)
        {
            delete[] copy;
        }

        return;
    }

    new_item->setText(szText);
    m_Content->addItem(new_item);

    while (m_Content->count() > 200)
    {
        auto first_item = m_Content->takeItem(0);
        delete first_item;
    }

    if (use_copy)
    {
        delete[] copy;
    }

    m_Content->scrollToBottom();
}

bool DebugConsole::is_open() const
{
    return !isHidden();
}

bool DebugConsole::is_docked() const
{
    return false;
}

void DebugConsole::print_raw_external(const char *szText)
{
    if (m_ExternalLocked && !m_WaitForLock)
    {
        return;
    }

    while (m_ExternalLocked)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    m_ExternalLocked = true;

    m_ExternalDataBuffer.push_back(szText);

    m_ExternalLocked = false;
}

void DebugConsole::update_external()
{
    if (m_ExternalLocked && !m_WaitForLock)
    {
        return;
    }

    while (m_ExternalLocked)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    m_ExternalLocked = true;

    if (!m_ExternalDataBuffer.size())
    {
        m_ExternalLocked = false;

        return;
    }

    // initialize "new" vector object to prevent lnt-accidental-copy
    // reference won't work because the vector needs to be cleared asap
    // so that another thread can continue writing to it
    // thus a copy is needed before processing the data
    auto cpy = std::vector(m_ExternalDataBuffer);

    m_ExternalDataBuffer.clear();

    m_ExternalLocked = false;

    if (!m_Content)
    {
        return;
    }

    for (const auto &i : cpy)
    {
        print_raw(i.c_str());
    }

    m_Content->scrollToBottom();
}

void DebugConsole::dock()
{
}

void DebugConsole::dock(int direction)
{
}

void DebugConsole::copy_data()
{
    auto selected = m_Content->selectedItems();

    if (!selected.isEmpty())
    {
        std::vector<QListWidgetItem *> selection_sorted;
        for (int i = 0; i < m_Content->count(); ++i)
        {
            QListWidgetItem *it = m_Content->item(i);
            if (it->isSelected())
            {
                selection_sorted.push_back(it);
            }
        }

        QString cb_data;
        for (const auto &i : selection_sorted)
        {
            cb_data += i->text() + "\n";
        }

        if (cb_data != m_OldSelection)
        {
            qApp->clipboard()->setText(cb_data);
            printf("Copied selection to clipboard.\n");
            m_OldSelection = cb_data;
        }
    }
}

int DebugConsole::get_dock_index() const
{
    return -1;
}

int DebugConsole::get_old_dock_index() const
{
    return -1;
}

void DebugConsole::ImTheTrashMan(const wchar_t *expression, const wchar_t *function, const wchar_t *file, unsigned int line, uintptr_t pReserved)
{
    UNREFERENCED_PARAMETER(expression);
    UNREFERENCED_PARAMETER(function);
    UNREFERENCED_PARAMETER(file);
    UNREFERENCED_PARAMETER(line);
    UNREFERENCED_PARAMETER(pReserved);

    // take that, CRT
    // but for real, the CRT error "handlers" are the dumbest shit ever because other than some strings you get no info to actually handle the error
    // or I am too dumb
    // probably both
}

bool DebugConsole::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusOut)
    {
        m_OldSelection = "";
    }
    else if (event->type() == QEvent::FocusIn)
    {
    }

    return QObject::eventFilter(obj, event);
}

void __stdcall g_print_to_console_raw(const char *szText)
{
    if (g_Console)
    {
        g_Console->print_raw(szText);
    }
    else
    {
        printf(szText);
    }
}

void __stdcall g_print_to_console_raw_external(const char *szText)
{
    if (g_Console)
    {
        g_Console->print_raw_external(szText);
    }
    else
    {
        printf(szText);
    }
}