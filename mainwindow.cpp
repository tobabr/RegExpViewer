#include <string>
#include <regex>

#include <QTextCursor>
#include <QBrush>
#include <QList>
#include <QString>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>

#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    treeModel = new QStandardItemModel;
    ui->treeView->setModel(treeModel);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QTextEdit::ExtraSelection createSelection(int &currentPos, std::smatch &match, const QTextCursor &cursor)
{
    QBrush yellow(Qt::yellow);
    QTextEdit::ExtraSelection select;

    select.cursor = QTextCursor(cursor);
    currentPos += match.position();
    select.cursor.setPosition(currentPos);
    select.cursor.setPosition(currentPos + match.length(), QTextCursor::KeepAnchor);
    currentPos += match.length();

    select.format.setBackground(yellow);

    return select;
}


QStandardItem* createTreeItem(std::smatch& match)
{
    QStandardItem* item = new QStandardItem(QString::fromStdString(match.str()));

    if(match.size() > 1) {
        QList<QStandardItem*> subItems{};

        for(int i = 1; i <= match.size(); ++i)
            subItems.push_back(new QStandardItem(QString::fromStdString(match[i])));

        item->appendColumn(subItems);
    }
    return item;
}


void MainWindow::on_lineEdit_textChanged(const QString &searchString)
{
    QList<QTextEdit::ExtraSelection> selections{};

    // Reset all visuals if the search field is empty
    if(searchString.length() == 0) {
        ui->lineEdit->setStyleSheet("background-color:white;");
        ui->textEdit->setExtraSelections(selections);
        treeModel->clear();
        return;
    }


    QList<QStandardItem *> treeItems{};
    std::string subject(ui->textEdit->toPlainText().toStdString());

    try {
        std::regex re(searchString.toStdString());
        std::sregex_iterator next(subject.begin(), subject.end(), re);
        std::sregex_iterator end;
        int currentPos = 0;

        while (next != end) {
            std::smatch match = *next;

        auto select = createSelection(currentPos, match, ui->textEdit->textCursor());
            auto item = createTreeItem(match);

            selections.push_back(select);
            treeItems.push_back(item);

            next++;
        }


        if(treeItems.length()) {
            treeModel->clear();
            treeModel->appendColumn(treeItems);
            treeModel->setHeaderData(0, Qt::Horizontal, QString("Matches"));
        }

        ui->textEdit->setExtraSelections(selections);
        ui->lineEdit->setStyleSheet("background-color:palegreen;");

    // Visualize regexp error
    } catch (std::regex_error& e) {
      ui->lineEdit->setStyleSheet("background-color:lightpink;");
      ui->textEdit->setExtraSelections(selections);
      treeModel->clear();
    }

}

void MainWindow::on_actionOpen_File_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(),
                tr("Text Files (*.txt);;C++ Files (*.cpp *.h);;All Files (*.*)"));

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return;
        }
        QTextStream in(&file);
        ui->textEdit->setText(in.readAll());
        file.close();
    }
}
