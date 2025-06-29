/*
 * Author:       Broihon
 * Copyright:    Guided Hacking™ © 2012-2023 Guided Hacking LLC
*/

#include "pch.h"

#include "StatusBox.h"

void StatusBox(bool ok, const QString & msg)
{
	if (ok)
		QMessageBox::information(nullptr, "Success", msg, QMessageBox::StandardButton::Ok);
	else
		QMessageBox::critical(nullptr, "Error", msg, QMessageBox::StandardButton::Ok);
}

bool YesNoBox(const QString & title, const QString & msg, QWidget * parent, QMessageBox::Icon icon)
{
	auto box = QMessageBox(icon, title, msg, QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
    box.setParent(parent);

	return (box.exec() == QMessageBox::StandardButton::Yes);
}