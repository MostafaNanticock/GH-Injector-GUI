/*
 * Author:       Broihon
 * Copyright:    Guided Hacking� � 2012-2023 Guided Hacking LLC
 */

#include "DotNetOptions.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

DotNetOptionsWindow::DotNetOptionsWindow(const QString &title, const QStringList &options, const DotNetOptionsTree *root, bool use_native,
                                         QWidget *parent) :
    QDialog(parent),
    cb_Entrypoint("Use native entrypoint")
{
    m_pOptionsRoot = root;

    auto *main_layout = new QVBoxLayout();
    auto *lay_namespace = new QHBoxLayout();
    auto *lay_classname = new QHBoxLayout();
    auto *lay_methodname = new QHBoxLayout();
    auto *lay_argument = new QHBoxLayout();
    auto *lay_options = new QHBoxLayout();

    lay_namespace->addWidget(new QLabel("Namespace:"), 0, Qt::AlignLeft);
    lay_classname->addWidget(new QLabel("Classname:"), 0, Qt::AlignLeft);
    lay_methodname->addWidget(new QLabel("Method:"), 0, Qt::AlignLeft);
    lay_argument->addWidget(new QLabel("Arguement:"), 0, Qt::AlignLeft);

    main_layout->addLayout(lay_namespace);
    main_layout->addLayout(lay_classname);
    main_layout->addLayout(lay_methodname);
    main_layout->addLayout(lay_argument);
    main_layout->addLayout(lay_options);

    if (root)
    {
        cmb_Namespace.setMinimumWidth(170);
        cmb_Classname.setMinimumWidth(170);
        cmb_Methodname.setMinimumWidth(170);

        lay_namespace->addWidget(&cmb_Namespace, 0, Qt::AlignRight);
        lay_classname->addWidget(&cmb_Classname, 0, Qt::AlignRight);
        lay_methodname->addWidget(&cmb_Methodname, 0, Qt::AlignRight);
    }
    else
    {
        txt_Namespace.setMinimumWidth(170);
        txt_Classname.setMinimumWidth(170);
        txt_Methodname.setMinimumWidth(170);

        txt_Namespace.setMaxLength(127);
        txt_Classname.setMaxLength(127);
        txt_Methodname.setMaxLength(127);

        lay_namespace->addWidget(&txt_Namespace, 0, Qt::AlignRight);
        lay_classname->addWidget(&txt_Classname, 0, Qt::AlignRight);
        lay_methodname->addWidget(&txt_Methodname, 0, Qt::AlignRight);

        QString alpha_numerical = "0-9a-zA-Z_";
        QString stupid_characters = "\\x{B5}";
        QString special_characters = "\\x{C0}-\\x{D6}\\x{D8}-\\x{F6}\\x{F8}-\\x{FF}";
        auto *validator = new QRegularExpressionValidator(QRegularExpression("[" + alpha_numerical + stupid_characters + special_characters + "]+"));

        txt_Namespace.setValidator(validator);
        txt_Classname.setValidator(validator);
        txt_Methodname.setValidator(validator);
    }

    txt_Argument.setText(options[3]);
    txt_Argument.setMaxLength(127);
    txt_Argument.setMinimumWidth(170);
    lay_argument->addWidget(&txt_Argument, 0, Qt::AlignRight);

    if (m_pOptionsRoot)
    {
        for (const auto &i : m_pOptionsRoot->GetOptions())
        {
            cmb_Namespace.addItem(i->GetData());
        }

        auto idx = cmb_Namespace.findText(options[0], Qt::MatchFixedString);
        if (idx == -1)
        {
            idx = 0;
        }
        cmb_Namespace.setCurrentIndex(idx);

        auto node_namespace = m_pOptionsRoot->Search(cmb_Namespace.currentText());
        for (const auto &i : node_namespace->GetOptions())
        {
            cmb_Classname.addItem(i->GetData());
        }

        idx = cmb_Classname.findText(options[1], Qt::MatchFixedString);
        if (idx == -1)
        {
            idx = 0;
        }
        cmb_Classname.setCurrentIndex(idx);

        auto node_classname = node_namespace->Search(cmb_Classname.currentText());
        for (const auto &i : node_classname->GetOptions())
        {
            cmb_Methodname.addItem(i->GetData());
        }

        idx = cmb_Methodname.findText(options[2], Qt::MatchFixedString);
        if (idx == -1)
        {
            idx = 0;
        }
        cmb_Methodname.setCurrentIndex(idx);
    }
    else
    {
        txt_Namespace.setText(options[0]);
        txt_Classname.setText(options[1]);
        txt_Methodname.setText(options[2]);
    }

    txt_Argument.setText(options[3]);

    auto *btn_save = new QPushButton("Save");
    lay_options->addWidget(btn_save, 0, Qt::AlignLeft);

    lay_options->addWidget(&cb_Entrypoint, 0, Qt::AlignRight);
    cb_Entrypoint.setFixedWidth(170);

    if (use_native)
    {
        m_UseNative = use_native;
        cb_Entrypoint.setChecked(true);

        if (m_pOptionsRoot)
        {
            cmb_Namespace.setDisabled(true);
            cmb_Classname.setDisabled(true);
            cmb_Methodname.setDisabled(true);
        }
        else
        {
            txt_Namespace.setDisabled(true);
            txt_Classname.setDisabled(true);
            txt_Methodname.setDisabled(true);
        }

        txt_Argument.setDisabled(true);
    }

    connect(&cmb_Namespace, SIGNAL(currentIndexChanged(int)), this, SLOT(on_namespace_change(int)));
    connect(&cmb_Classname, SIGNAL(currentIndexChanged(int)), this, SLOT(on_classname_change(int)));
    connect(btn_save, SIGNAL(clicked()), this, SLOT(on_save_button_clicked()));
    connect(&cb_Entrypoint, SIGNAL(clicked()), this, SLOT(on_native_changed()));

    setLayout(main_layout);

    setWindowIcon(QIcon(":/GuiMain/gh_resource/GH Icon.ico"));
    setWindowTitle(title);

    installEventFilter(this);
}

void DotNetOptionsWindow::GetResults(std::vector<QString> &results, bool &use_native)
{
    results = m_Results;
    use_native = m_UseNative;
}

void DotNetOptionsWindow::GetResult(QString &result, std::uint32_t index)
{
    if (index < m_Results.size())
    {
        result = m_Results[index];
    }
}

void DotNetOptionsWindow::on_save_button_clicked()
{
    m_Results.clear();

    if (m_pOptionsRoot)
    {
        m_Results.push_back(cmb_Namespace.currentText());
        m_Results.push_back(cmb_Classname.currentText());
        m_Results.push_back(cmb_Methodname.currentText());
    }
    else
    {
        m_Results.push_back(txt_Namespace.text());
        m_Results.push_back(txt_Classname.text());
        m_Results.push_back(txt_Methodname.text());
    }

    m_Results.push_back(txt_Argument.text());

    m_UseNative = cb_Entrypoint.isChecked();

    close();
}

void DotNetOptionsWindow::on_native_changed()
{
    m_UseNative = cb_Entrypoint.isChecked();

    if (m_pOptionsRoot)
    {
        cmb_Namespace.setEnabled(!m_UseNative);
        cmb_Classname.setEnabled(!m_UseNative);
        cmb_Methodname.setEnabled(!m_UseNative);
    }
    else
    {
        txt_Namespace.setEnabled(!m_UseNative);
        txt_Classname.setEnabled(!m_UseNative);
        txt_Methodname.setEnabled(!m_UseNative);
    }

    txt_Argument.setEnabled(!m_UseNative);
}

void DotNetOptionsWindow::on_namespace_change(int index)
{
    Q_UNUSED(index);

    cmb_Classname.clear();

    auto node_namespace = m_pOptionsRoot->Search(cmb_Namespace.currentText());
    for (const auto &i : node_namespace->GetOptions())
        cmb_Classname.addItem(i->GetData());

    cmb_Classname.setCurrentIndex(0);
}

void DotNetOptionsWindow::on_classname_change(int index)
{
    Q_UNUSED(index);

    if (!cmb_Classname.count())
        return;

    cmb_Methodname.clear();

    auto node_namespace = m_pOptionsRoot->Search(cmb_Namespace.currentText());
    auto node_classname = node_namespace->Search(cmb_Classname.currentText());
    for (const auto &i : node_classname->GetOptions())
        cmb_Methodname.addItem(i->GetData());
}

bool DotNetOptionsWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        auto keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
        {
            on_save_button_clicked();
        }
    }
    else if (event->type() == QEvent::Show)
    {
        if (m_pOptionsRoot)
        {
            txt_Argument.setFocus();
        }
        else
        {
            txt_Namespace.setFocus();
        }
    }

    return QObject::eventFilter(obj, event);
}

DotNetOptionsTree::DotNetOptionsTree()
{
    m_Option = QString();
}

DotNetOptionsTree::DotNetOptionsTree(const QString &option)
{
    m_Option = option;
}

DotNetOptionsTree::~DotNetOptionsTree()
{
    for (auto &i : m_Nodes)
        delete i;
}

void DotNetOptionsTree::ParseData(const QString &Data)
{
    auto full_list = Data.split('!', Qt::SkipEmptyParts);
    for (const auto &i : full_list)
    {
        auto individual_items = i.split(';', Qt::SkipEmptyParts);
        if (individual_items.size() != 3)
        {
            continue;
        }

        auto &new_namespace = individual_items[0];
        auto &new_classname = individual_items[1];
        auto &new_methodname = individual_items[2];

        auto node_namespace = this->Search(new_namespace);
        if (!node_namespace)
        {
            node_namespace = new DotNetOptionsTree(new_namespace);
            m_Nodes.push_back(node_namespace);
        }

        auto node_classname = node_namespace->Search(new_classname);
        if (!node_classname)
        {
            node_classname = new DotNetOptionsTree(new_classname);
            node_namespace->m_Nodes.push_back(node_classname);
        }

        auto node_methodname = node_classname->Search(new_methodname);
        if (!node_methodname)
        {
            node_methodname = new DotNetOptionsTree(new_methodname);
            node_classname->m_Nodes.push_back(node_methodname);
        }
    }
}

const std::vector<DotNetOptionsTree *> DotNetOptionsTree::GetOptions() const
{
    return m_Nodes;
}

const QString &DotNetOptionsTree::GetData() const
{
    return m_Option;
}

DotNetOptionsTree *DotNetOptionsTree::Search(const QString &option) const
{
    for (const auto &i : m_Nodes)
    {
        if (option.compare(i->m_Option, Qt::CaseSensitive) == 0)
        {
            return i;
        }
    }

    return nullptr;
}