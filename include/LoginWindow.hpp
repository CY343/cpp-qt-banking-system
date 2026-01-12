#ifndef LOGINWINDOW_HPP
#define LOGINWINDOW_HPP

#include <QDialog>
#include <memory>
#include "Services.hpp"
#include "Customers.hpp"
#include"BankAccount.hpp"

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr, Services* services = nullptr);
    ~LoginWindow();

    std::shared_ptr<Customers> getLoggedInCustomer() const { return loggedInCustomer_; }

private slots:
    void handleLogin();

private:
    Ui::LoginWindow *ui;
    Services* services_;  // pointer to your backend service
    std::shared_ptr<Customers> loggedInCustomer_; // store logged-in customer

    bool checkCredentials(const std::string& username, const std::string& password);
};

#endif // LOGINWINDOW_HPP
