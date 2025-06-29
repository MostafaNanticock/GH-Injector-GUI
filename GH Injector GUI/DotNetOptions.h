/*
 * Author:       Broihon
 * Copyright:    Guided Hacking™ © 2012-2023 Guided Hacking LLC
 */

#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QLineEdit>

class DotNetOptionsTree
{
    QString m_Option;
    std::vector<DotNetOptionsTree *> m_Nodes;

public:
    DotNetOptionsTree();
    explicit DotNetOptionsTree(const QString &option);
    ~DotNetOptionsTree();

    void ParseData(const QString &Data);

    const std::vector<DotNetOptionsTree *> GetOptions() const;
    const QString &GetData() const;

    DotNetOptionsTree *Search(const QString &option) const;
};

class DotNetOptionsWindow : public QDialog
{
    Q_OBJECT

    const DotNetOptionsTree *m_pOptionsRoot;

    QComboBox cmb_Namespace;
    QComboBox cmb_Classname;
    QComboBox cmb_Methodname;

    QLineEdit txt_Namespace;
    QLineEdit txt_Classname;
    QLineEdit txt_Methodname;

    QLineEdit txt_Argument;

    QCheckBox cb_Entrypoint;

    std::vector<QString> m_Results;
    bool m_UseNative = false;

public:
    DotNetOptionsWindow(const QString &title, const QStringList &options, const DotNetOptionsTree *root, bool use_native = false,
                        QWidget *parent = nullptr);

    void GetResults(std::vector<QString> &results, bool &use_native);
    void GetResult(QString &result, std::uint32_t index);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void on_save_button_clicked();
    void on_native_changed();

    void on_namespace_change(int index);
    void on_classname_change(int index);
};
