/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez                                                     *
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

#include "tupinfowidget.h"
#include "tseparator.h"
#include "timagebutton.h"
#include "tpushbutton.h"
#include "tupwebhunter.h"

TupInfoWidget::TupInfoWidget(QWidget *parent) : QWidget(parent)
{
    currencyList << "ARS";
    currencyList << "AUD";
    currencyList << "BRL";
    currencyList << "CAD";
    currencyList << "CNY";
    currencyList << "COP";
    currencyList << "EUR";
    currencyList << "MXN";
    currencyList << "NZD";
    currencyList << "NIO";
    currencyList << "NOK";
    currencyList << "PAB";
    currencyList << "PEN";
    currencyList << "PKR";
    currencyList << "SEK";
    currencyList << "TWD";
    currencyList << "USD";
    currencyList << "UYU";

    currentCurrency = currencyList.at(currencyList.indexOf("USD"));

    QBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(2);

    innerLayout = new QVBoxLayout;

    setUIContext();

    TImageButton *fileButton = new TImageButton(QPixmap(THEME_DIR + "icons/open_big.png"), 60, this, true);
    connect(fileButton, SIGNAL(clicked()), this, SLOT(loadFile()));

    TImageButton *linksButton = new TImageButton(QPixmap(THEME_DIR + "icons/links_big.png"), 60, this, true);
    connect(linksButton, SIGNAL(clicked()), this, SLOT(showLinkPanel()));

    TImageButton *closeButton = new TImageButton(QPixmap(THEME_DIR + "icons/close_big.png"), 60, this, true);
    closeButton->setDefault(true);
    connect(closeButton, SIGNAL(clicked()), this, SIGNAL(closePanel()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    buttonBox->addButton(fileButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(linksButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(closeButton, QDialogButtonBox::ActionRole);

    innerLayout->addWidget(new TSeparator());
    innerLayout->addWidget(buttonBox);

    layout->addLayout(innerLayout);
}

TupInfoWidget::~TupInfoWidget()
{
}

void TupInfoWidget::setUIContext()
{
    table = new QTableWidget(currencyList.count() - 1, 2);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->horizontalHeader()->hide();
    table->verticalHeader()->hide();

    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    table->setMaximumWidth(250);
    table->setMaximumHeight((currencyList.count() - 1)*30);

    // table->verticalHeader()->setResizeMode(QHeaderView::Stretch);
    // table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(1, 1, 1, 1);
    mainLayout->setSpacing(5);

    QLabel *titleLabel = new QLabel(tr("Currency Converter"));
    QFont font = this->font();
    font.setPointSize(12);
    font.setBold(true);
    titleLabel->setFont(font);
    // titleLabel->setFont(QFont("Arial", 12, QFont::Bold, false));
    titleLabel->setAlignment(Qt::AlignHCenter);

    QLabel *currencyLabel = new QLabel(tr("Currency"));
    QComboBox *currency = new QComboBox();

    for (int i=0; i<currencyList.count(); i++)
         currency->addItem(tr("%1").arg(currencyList.at(i)));

    connect(currency, SIGNAL(currentIndexChanged(int)), this, SLOT(setCurrentCurrency(int)));
    currency->setCurrentIndex(currencyList.indexOf("USD"));

    currencyLabel->setBuddy(currency);

    QHBoxLayout *currencyLayout = new QHBoxLayout;
    currencyLayout->addWidget(currencyLabel);
    currencyLayout->addWidget(currency);

    QLabel *sourceLabel = new QLabel(tr("Source"));
    QLineEdit *source = new QLineEdit("http://www.webservicex.net");

    updateMoneyTable();

    QHBoxLayout *sourceLayout = new QHBoxLayout;
    sourceLayout->addWidget(sourceLabel);
    sourceLayout->addWidget(source);

    QLabel *checkerLabel = new QLabel(tr("Update data every"));
    QComboBox *minutesCombo = new QComboBox();

    minutesCombo->addItem(tr("1") + " " + tr("minute"));
    for (int i=5; i< 20; i+=5)
         minutesCombo->addItem(tr("%1").arg(i) + " " + tr("minutes"));

    QHBoxLayout *checkerLayout = new QHBoxLayout;
    checkerLayout->addWidget(checkerLabel);
    checkerLayout->addWidget(minutesCombo);

    mainLayout->addWidget(titleLabel);
    mainLayout->addLayout(currencyLayout);
    mainLayout->addLayout(sourceLayout);
    mainLayout->addWidget(table);
    mainLayout->addLayout(checkerLayout);

    innerLayout->addLayout(mainLayout);

    getDataFromNet();
}

void TupInfoWidget::getDataFromNet()
{
    for (int i=0; i<currencyList.count(); i++) {
         if (currencyList.at(i).compare(currentCurrency) != 0)
             getCurrencyConversionFromNet(currentCurrency, currencyList.at(i));
    }
}

void TupInfoWidget::getCurrencyConversionFromNet(const QString &money1, const QString &money2)
{
    QList<QString> params;
    params << money1;
    params << money2;
    QString url = "http://www.webservicex.net//currencyconvertor.asmx/ConversionRate?FromCurrency=1&ToCurrency=2";

    // SQA: Experimental code
    /*
    TupWebHunter *hunter = new TupWebHunter(TupWebHunter::Currency, url, params);
    hunter->start();
    connect(hunter, SIGNAL(dataReady(const QString &)), this, SLOT(updateObjectInformation(const QString &)));
    */
}

void TupInfoWidget::updateObjectInformation(const QString &data)
{
    QStringList parts = data.split(":"); 
    QString currency = parts.at(0);
    QString value = parts.at(1);

    for (int i=0; i < table->rowCount(); i++) {
         QTableWidgetItem *item = table->item(i, 0);
         QString label = item->text();
         if (label.compare(currency) == 0) {
             double number = value.toDouble();
             if (number <= 0) {
                 value = "UNAVAILABLE";
             }
             QTableWidgetItem *label = new QTableWidgetItem("  " + tr("%1").arg(value));
             table->setItem(i, 1, label);
         }
    }
}

void TupInfoWidget::setCurrentCurrency(int index)
{
    currentCurrency = currencyList.at(index);
    updateMoneyTable();
}

void TupInfoWidget::updateMoneyTable()
{
    int j = 0;
    for (int i=0; i<currencyList.count(); i++) {
         if (currencyList.at(i).compare(currentCurrency) != 0) {
             QTableWidgetItem *label = new QTableWidgetItem(tr("%1").arg(currencyList.at(i)));
             label->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
             QTableWidgetItem *empty = new QTableWidgetItem(tr(""));
             empty->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

             table->setItem(j, 0, label);
             table->setItem(j, 1, empty);
             j++;
         }
    }

    getDataFromNet();
}

void TupInfoWidget::loadFile()
{
    const char *home = getenv("HOME");

    QString package = QFileDialog::getOpenFileName(this, tr("Link file to Object"), home,
                      tr("All files (*.*)"));

    if (package.isEmpty())
        return;
}
