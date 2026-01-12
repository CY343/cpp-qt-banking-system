#include "managerwindow.h"
#include "ui_managerwindow.h"
#include"Services.hpp"
#include"Customers.hpp"
#include"QMessageBox"
#include"QInputDialog"
#include"QDebug"
#include<QTableWidget>
#include<string>


ManagerWindow::ManagerWindow(QWidget *parent, Services* services)
    : QMainWindow(parent)
    , ui(new Ui::ManagerWindow)
    , services_(services)
{
    ui->setupUi(this);
    this->setWindowTitle("System Manager Dashboard");
    setWindowIcon(QIcon(":/icons/resources/icons/Bank of World.ico"));



    // connect signals to slots
    connect(ui->customerTableWidget, &QTableWidget::cellClicked, this, &ManagerWindow::on_customerTableWidget_cellClicked);

    refreshCustomerList();


}

ManagerWindow::~ManagerWindow()
{
    delete ui;
}

// =============== core functions =================
void ManagerWindow::refreshCustomerList(){
    if(!services_) return;

    currentCustomerList_ = services_->getCustomers();
    ui->customerTableWidget->setRowCount(currentCustomerList_.size());
    ui->customerTableWidget->setColumnCount(4);

    QStringList headers = {"Name", "Email", "Age", "CustomerId(Short)"};
    ui->customerTableWidget->setHorizontalHeaderLabels(headers);
    ui->customerTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    for(int row = 0; row < currentCustomerList_.size(); ++row){
        auto& c = currentCustomerList_[row];

        ui->customerTableWidget->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(c->getName())));
        ui->customerTableWidget->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(c->getEmail())));
        ui->customerTableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(c->getAge())));
        // display the short verison of UUID
        ui->customerTableWidget->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(c->getShortCustomerId())));

    }
    ui->customerTableWidget->resizeColumnsToContents();

}

void ManagerWindow::displayCustomerAccounts(const std::string& customerId){
    // find the customer associated with the clicked id
    auto it = std::find_if(currentCustomerList_.begin(), currentCustomerList_.end(),
                                 [&customerId](const std::shared_ptr<Customers>& c){
        return c->getCustomerId() == customerId;
    });
    if(it == currentCustomerList_.end()) return;

    // get the accounts from selected customer
    const auto& accounts = (*it)->getAccount();
    ui->accountDetailTable->clear();
    ui->accountDetailTable->setRowCount(accounts.size());
    ui->accountDetailTable->setColumnCount(3);
    ui->accountDetailTable->setHorizontalHeaderLabels({"Account #", "Type", "Balance"});

    for(int row = 0; row < accounts.size(); ++ row){
    auto& acc = accounts[row];
    // system view to access
    QString type = std::dynamic_pointer_cast<SavingAccount>(acc)? "Saving":"Checking";
    ui->accountDetailTable->setItem(row, 0, new QTableWidgetItem(QString::number(acc->getAccountNumber())));
    ui->accountDetailTable->setItem(row, 1, new QTableWidgetItem(type));
    ui->accountDetailTable->setItem(row, 2, new QTableWidgetItem(QString::number(acc->getAccountBalance(), 'f', 2)));
}

    ui->accountDetailTable->resizeColumnsToContents();

}

void ManagerWindow::on_customerTableWidget_cellClicked(int row, int column){
    if(row < 0 || row > currentCustomerList_.size()) return;
    const auto& customer = currentCustomerList_[row];
    displayCustomerAccounts(customer->getCustomerId());
}


void ManagerWindow::on_addCustomerButton_clicked()
{
    bool ok;

    // Prompt for required details
    QString name = QInputDialog::getText(this, "New Customer Registration", "Name:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;

    QString email = QInputDialog::getText(this, "New Customer Registration", "Email:", QLineEdit::Normal, "", &ok);
    if (!ok || email.isEmpty()) return;

    QString phone = QInputDialog::getText(this, "New Customer Registration", "Phone Number (10 digits):", QLineEdit::Normal, "", &ok);
    if (!ok || phone.isEmpty()) return;

    int age = QInputDialog::getInt(this, "New Customer Registration", "Age:", 18, 0, 150, 1, &ok);
    if (!ok) return;

    QString address = QInputDialog::getText(this, "New Customer Registration", "Address:", QLineEdit::Normal, "", &ok);
    if (!ok || address.isEmpty()) return;

    try {
        // Create the new Customer object (Model layer)
        auto newCustomer = std::make_shared<Customers>(
            name.toStdString(),
            email.toStdString(),
            phone.toStdString(),
            age,
            address.toStdString()
            );

        // Add customer to the system (Service layer)
        services_->addCustomers({newCustomer});

        // Add a default account for the manager to view
        auto defaultAccount = std::make_shared<SavingAccount>(100.0, 0.01);
        newCustomer->addAccount(defaultAccount);

        QMessageBox::information(this, "Success", "New customer registered and default account created.");
        refreshCustomerList();
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Registration Failed", QString("Error: %1").arg(e.what()));
    }
}

// --- In ManagerWindow::on_deleteCustomerButton_clicked() ---

void ManagerWindow::on_deleteCustomerButton_clicked()
{
    int row = ui->customerTableWidget->currentRow();
    if (row < 0 || row >= currentCustomerList_.size()) {
        QMessageBox::warning(this, "Action Failed", "Please select a customer to delete.");
        return;
    }

    // 1. Retrieve the shared_ptr<Customers> object directly from the list.
    // This is the pointer the Services function requires.
    const std::shared_ptr<Customers>& customerToDelete = currentCustomerList_[row];

    std::string customerName = customerToDelete->getName();

    // Ask for confirmation (Essential for destructive actions)
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Deletion",
                                                              QString("Are you sure you want to permanently delete customer '%1' and ALL associated accounts?").arg(QString::fromStdString(customerName)),
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 2. Call Services::deleteCustomers, passing the shared_ptr object.
        // This resolves the LNK2019 conversion error.
        if (services_->deleteCustomers(customerToDelete)) {
            QMessageBox::information(this, "Success", QString("Customer '%1' successfully deleted from the system.").arg(QString::fromStdString(customerName)));
            refreshCustomerList();
            ui->accountDetailTable->clearContents(); // Clear the table data
            ui->accountDetailTable->setRowCount(0);  // Reset row count to zero

            QMessageBox::information(this, "Success",
                                     QString("Customer '%1' successfully deleted from the system.").arg(QString::fromStdString(customerName)));
        } else {
            QMessageBox::critical(this, "Deletion Failed", "Failed to remove customer from the service registry.");
        }
    }

}
