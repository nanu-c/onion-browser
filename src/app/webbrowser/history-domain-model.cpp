/*
 * Copyright 2013 Canonical Ltd.
 *
 * This file is part of webbrowser-app.
 *
 * webbrowser-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * webbrowser-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "domain-utils.h"
#include "history-domain-model.h"
#include "history-model.h"
#include "history-timeframe-model.h"

// Qt
#include <QtCore/QUrl>

/*!
    \class HistoryDomainModel
    \brief Proxy model that filters the contents of a history model
           based on a domain name

    HistoryDomainModel is a proxy model that filters the contents of a
    history model based on a domain name.

    An entry in the history model matches if the domain name extracted from
    its URL equals the filter domain name (case-insensitive comparison).

    When no domain name is set (null or empty string), all entries match.
*/
HistoryDomainModel::HistoryDomainModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, SIGNAL(layoutChanged(QList<QPersistentModelIndex>, QAbstractItemModel::LayoutChangeHint)), SLOT(onModelChanged()));
    connect(this, SIGNAL(modelReset()), SLOT(onModelChanged()));
    connect(this, SIGNAL(rowsInserted(QModelIndex, int, int)), SLOT(onModelChanged()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex, int, int)), SLOT(onModelChanged()));
    connect(this, SIGNAL(dataChanged(QModelIndex, QModelIndex, QVector<int>)), SLOT(onModelChanged()));
}

HistoryTimeframeModel* HistoryDomainModel::sourceModel() const
{
    return qobject_cast<HistoryTimeframeModel*>(QSortFilterProxyModel::sourceModel());
}

void HistoryDomainModel::setSourceModel(HistoryTimeframeModel* sourceModel)
{
    if (sourceModel != this->sourceModel()) {
        QSortFilterProxyModel::setSourceModel(sourceModel);
        Q_EMIT sourceModelChanged();
    }
}

const QString& HistoryDomainModel::domain() const
{
    return m_domain;
}

void HistoryDomainModel::setDomain(const QString& domain)
{
    if (domain != m_domain) {
        m_domain = domain;
        invalidate();
        Q_EMIT domainChanged();
    }
}

const QDateTime& HistoryDomainModel::lastVisit() const
{
    return m_lastVisit;
}

bool HistoryDomainModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (m_domain.isEmpty()) {
        return true;
    }
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    QUrl url = sourceModel()->data(index, HistoryModel::Url).toUrl();
    QString domain = DomainUtils::extractTopLevelDomainName(url);
    return (domain.compare(m_domain, Qt::CaseInsensitive) == 0);
}

void HistoryDomainModel::onModelChanged()
{
    if (rowCount() > 0) {
        m_lastVisit = data(index(0, 0), HistoryModel::LastVisit).toDateTime();
    } else {
        m_lastVisit = QDateTime();
    }
    Q_EMIT lastVisitChanged();
}