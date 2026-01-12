   /**
    * @class CheckingAccount
    * @brief Represents a checking account derived from the BankAccount class.
    * 
    * This class extends core banking functionality by adding checking account-specific
    * features such as overdraft protection, monthly maintenance fees, transaction tracking,
    * and debit/credit card management. It also supports business account designation and
    * customizable withdrawal limits.
    * 
    * @details 
    * Key features include:
    * - Overdraft limit management and optional overdraft protection via a backup account.
    * - Monthly maintenance fee with optional waiver based on minimum balance.
    * - Linked debit and credit card management.
    * - Daily ATM/cash withdrawal limits.
    * - Tracking of monthly transaction counts.
    * - Ability to convert the account to a business account type.
    * 
    * @note Inherits from the BankAccount base class and overrides certain methods
    * such as applyWithdraw() to implement account-specific transaction behavior.
    */
   #ifndef CHECKINGACCOUNT_HPP
   #define CHECKINGACCOUNT_HPP
   #include"BankAccount.hpp"
   #include"DebitCard.hpp"
   #include"CreditCard.hpp"
   #include<memory>

   class CheckingAccount:public BankAccount
   {   
   private:
   // core checking account features

   /**
   * @brief The maximum overdraft amount allowed for the account.
   * 
   * Transactions that exceed the current balance up to this limit may still be approved
   * if overdraft protection is enabled.
   */
   double overdraft_limit_;

   /**
   * @brief The monthly maintenance fee charged to the account.
   * 
   * This fee may be waived if the account maintains a minimum required balance.
   */
   double monthly_maintenance_fee_;

   /**
   * @brief The minimum account balance required to waive the monthly maintenance fee.
   */
   double minimum_balance_waiver_;

   /**
   * @brief Indicates whether overdraft protection is enabled for the account.
   * 
   * If true, transactions exceeding the balance up to the overdraft limit (or a linked backup account)
   * may still be processed.
   */
   bool has_overdraft_protection_;

   /**
   * @brief The number of transactions performed in the current month.
   * 
   * Used to track free or fee-based transactions.
   */
   int monthly_transaction_count_; 

   /**
   * @brief List of debit cards associated with this checking account.
   */
   //std::vector<std::shared_ptr<DebitCard>> debit_cards_;


   /**
   * @brief List of credit cards associated with this checking account.
   */
   std::vector<std::shared_ptr<CreditCard>> credit_cards_;

   /** 
   *  @brief List of debit cards associated with this checking account.
   */
   std::vector<std::shared_ptr<DebitCard>> debit_cards_;

   /**
    * @brief Checks if the account is a business account.
    *
    * @return true If the account is designated as a business account.
    * @return false Otherwise.
    */
   bool is_business_account_; 



   // Security feature
   // ATM or cash withdrawal limit
   /**
   * @brief Maximum amount allowed for ATM or cash withdrawals in a single day.
   */
   double daily_withdrawal_limit_;

   // Account type
   /**
   * @brief Flag indicating whether this account is a business account.
   */
   //bool is_business_account_;

   /**
   * @brief A weak pointer to a backup bank account for overdraft protection.
   * 
   * If overdraft protection is enabled, this backup account may be used
   * to cover transactions exceeding the account balance.
   */
   std::weak_ptr<BankAccount> overdraft_backup_account_; 

   double initial_balance_;

   protected:

   /**
   * @brief Notification hook called when a maintenance fee is waived.
   * 
   * This method can be overridden or extended by derived classes to provide
   * custom behavior or logging when the account qualifies for a maintenance fee waiver.
   */
   virtual void onFeeWaived() const;  // Notification hook

   public:
      /**
    * @brief Constructs a new CheckingAccount object with optional configuration.
    * 
    * @param overdraft_limit The maximum allowed overdraft amount. Default is 0.0.
    * @param monthly_fee The monthly maintenance fee for the account. Default is 10.0.
    * @param fee_wavier_balance The minimum balance to qualify for a fee waiver. Default is 1500.0.
    * @param daily_withdrawal_limit The daily ATM/cash withdrawal limit. Default is 500.0.
    */
   CheckingAccount(double overdraft_limit = 0.0, 
                     double monthly_fee = 10.0, 
                     double fee_wavier_balance = 1500.00,
                     double daily_withdrawal_limit = 500.0,
                     double initial_balance = 0.0);
               
   /**
    * @brief Constructs a CheckingAccount using an existing account identifier.
    *
    * This constructor is intended for restoring or reconstructing accounts
    * that were previously created and already have an assigned account number.
    * Typical use cases include loading accounts from persistent storage,
    * importing external data, or controlled testing scenarios.
    *
    * @param id Existing account number to assign to this account.
    * @param balance Initial account balance.
    * @param accountType Account type label (e.g., "CHECKING").
    *
    * @note This constructor does not generate a new account number.
    * @warning Callers are responsible for ensuring the provided account number
    *          is unique within the system.
    */
   CheckingAccount(int id, double balance, const std::string& accountType);


   // overdraft management
   /**
    * @brief Sets a new overdraft limit for the account.
    * 
    * @param limit The new overdraft limit value.
    * @return True if the limit was successfully set, false otherwise.
    */
   bool setOverdraftLimit(double limit);

      /**
    * @brief Retrieves the current overdraft limit.
    * 
    * @return The overdraft limit value.
    */
   double getOverdraftLimit() const;

   // Fee handling 
   /**
    * @brief Applies the monthly maintenance fee to the account if applicable.
    * 
    * If the account qualifies for a fee waiver based on its balance, no fee is applied.
    */            
   void applyMonthlyMaintenanceFee();

   /**
    * @brief Checks if the account qualifies for a maintenance fee waiver.
    * 
    * @return True if the current balance meets or exceeds the waiver threshold, false otherwise.
    */           
   bool qualifiesForFeeWaiver() const;

   // Debit card operations.
   // Issue a debit card
   /**
    * @brief Issues a new debit card associated with this account.
    * 
    * @param card_number The card number.
    * @param expiration The expiration date in MM/YY format.
    * @param cvv The card's CVV code.
    */           
   void issueDebitCard();

   /**
    * @brief Blocks a debit card associated with this checking account.
    * 
    * Searches for the debit card with the specified card number and
    * deactivates it if found. Marks the card as expired.
    * 
    * @param cardNumber The card number of the debit card to block.
    * @return true If the card was found and successfully blocked.
    * @return false If no matching card was found.
    */
   bool blockDebitCard(const std::string& cardNumber);

   // Transaction override
   /**
    * @brief Attempts to withdraw a specified amount from the account.
    * 
    * Overrides the base class withdraw method to include overdraft protection 
    * and daily withdrawal limit checks.
    * 
    * @param amount The amount to withdraw.
    * @return True if the withdrawal is successful, false otherwise.
    */
   bool applyWithdraw(const double& amount) override;

   // Account type management
   /**
    * @brief Converts the account to a business account or reverts it to personal.
    * 
    * 
    */
   void convertTOBusinessAccount();

   //------getters-------
   /**
    * @brief Retrieves the monthly maintenance fee.
    * 
    * @return The monthly fee amount.
    */
   double getMonthlyfee() const;


   /**
    * @brief Retrieves the daily ATM/cash withdrawal limit.
    * 
    * @return The daily withdrawal limit.
    */
   double getDailyWithdrawalLimit() const;

   /**
    * @brief Retrieves the number of transactions performed this month.
    * 
    * @return The transaction count.
    */           
   int getMonthlyTransactionCount() const;

   /**
    * @brief Adds a credit card to the account.
    * 
    * @param creditLimit The credit limit for the new card.
    * @param isActivated Whether the card is activated upon creation.
    * @param linked_account The bank account linked to the credit card.
    */
   void addCreditCard( double creditLimit, bool isActivated,std::shared_ptr<BankAccount> linked_account);
   //void addDebitCard(const std::string& cardnumber, const std::string& expiration, const std::string& cvv, bool isActivated, bool isExpired);
         
   // Overdraft protection methods
   /**
    * @brief Links a backup bank account for overdraft protection.
    * 
    * @param backup The backup BankAccount shared pointer.
    */
   void linkOverdraftProtection(std::shared_ptr<BankAccount> backup);

   /**
    * @brief Checks if overdraft protection is enabled.
    * 
    * @return True if protection is enabled, false otherwise.
    */
   bool hasOverdraftProtection() const;

   /**
    * @brief Retrieves the linked backup account for overdraft protection.
    * 
    * @return A shared pointer to the backup BankAccount.
    */
   std::shared_ptr<BankAccount> getBackupAccount() const;

   /**
    * @brief Applies monthly account processing for a checking account.
    *
    * Checking accounts do not accrue interest. This override repurposes the
    * interest hook to handle monthly maintenance logic:
    * - Waives the monthly maintenance fee if the balance meets the waiver threshold
    * - Otherwise applies the monthly maintenance fee as a transaction
    *
    * @note This method does not calculate interest.
    */
   void applyInterest() override;

   /**
    * @brief Issues a new credit card linked to this checking account.
    *
    * Creates and links a CreditCard object with the specified credit limit
    * and activation status. The checking account is used as the linked
    * backing account.
    *
    * @param creditLimit The credit limit for the new credit card.
    * @param isActivated Whether the credit card is activated upon issuance.
    *
    * @note Credit limit validation is enforced internally.
    */
      void issueCreditCard(double creditLimit, bool isActivated);
            
   /**
    * @brief Checks whether this checking account is designated as a business account.
    *
    * @return true if the account is marked as a business account, false otherwise.
    */
   bool isBusinessAccount() const;

   /**
    * @brief Sets the account balance directly.
    *
    * This function bypasses normal deposit/withdraw validation and should
    * be used with care. Intended for internal adjustments, testing, or
    * account restoration scenarios.
    *
    * @param amount The new balance to assign to the account.
    *
    * @warning Does not record a transaction or perform validation.
    */
   void setAccountBalance(double amount);

   /**
    * @brief Retrieves all debit cards associated with this checking account.
    *
    * @return A constant reference to the vector of shared pointers to DebitCard objects.
    */
   const std::vector<std::shared_ptr<DebitCard>>& getDebitCards() const;


   /**
    * @brief Retrieves all credit cards associated with this checking account.
    *
    * @return A constant reference to the vector of shared pointers to CreditCard objects.
    */
   const std::vector<std::shared_ptr<CreditCard>>& getCreditCards() const;
            
   };


   #endif