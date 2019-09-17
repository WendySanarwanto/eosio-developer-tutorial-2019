// #include <ctime>
#include <eosio/eosio.hpp>
#include "abcounter.cpp"

using namespace eosio;
using namespace std;

class [[eosio::contract("addressbook")]] addressbook: public contract {
    public:   
        // using contract::contract;
        addressbook(name receiver, name code, datastream<const char*> ds): contract(receiver, code, ds) {}

        [[eosio::action]]
        void upsert(
            name user,
            string first_name,
            string last_name,
            uint64_t age,
            string street,
            string city,
            string state
        ) {
            // only the user has control over their own record
            require_auth(user);
            // Instantiate address book table's index:
            // - 1st param: get_self() function which will pass the name of this contract.
            // - 2nd param: get_first_receiver is the account name this contract is deployed to.
            address_index addresses(get_self(), get_first_receiver().value);
            // Table's rows iterator
            auto iterator = addresses.find(user.value);
            // detect whether or not a particular user already exists in the table
            if (iterator == addresses.end()) {
                // The user is not in the table
                // Create a record in the table using the multi_index's "emplace" method
                // This method accepts two arguments, the "payer" of this record 
                // who pays the storage usage and a callback function
                addresses.emplace(user, [&]( auto& row){
                    row.key = user;
                    row.first_name = first_name;
                    row.last_name = last_name;
                    row.age = age;
                    row.street = street;
                    row.city = city;
                    row.state = state;
                });
                send_summary(user, " successfully emplaced record to addressbook.");
                increment_counter(user, "emplace");
            } else {
                // The user exist in the table
                // update existing record by using the multi_index's "modify" method.
                addresses.modify(iterator, user, [&]( auto& row ){
                    row.key = user;
                    row.first_name = first_name;
                    row.last_name = last_name;
                    row.age = age;
                    row.street = street;
                    row.city = city;
                    row.state = state;                    
                });
                send_summary(user, " successfully modified record to addressbook.");
                increment_counter(user, "modified");
            }
        }

        [[eosio::action]]
        void erase(name user) {
            // only the user has control over their own record
            require_auth(user);
            // Instantiate address book table's index:
            // - 1st param: get_self() function which will pass the name of this contract.
            // - 2nd param: get_first_receiver is the account name this contract is deployed to.
            address_index addresses(get_self(), get_first_receiver().value);
            // Try to find matched user record 1st
            auto iterator = addresses.find(user.value);
            // Check if the target record exist
            check(iterator != addresses.end(), "Record does not exist.");
            // Erase the record
            addresses.erase(iterator);
            send_summary(user, " successfully erased record from addressbook.");
            increment_counter(user, "erased");
        }

        /**
         * Dispatches a "transaction receipt" whenever a transaction occurs
         */
        [[eosio::action]]
        void notify(name user, string msg) {
            require_auth(get_self());
            require_recipient(user);

        }

    private:
        struct [[eosio::table]] person {
            name key;
            string first_name;
            string last_name;
            uint64_t age;
            string street;
            string city;
            string state;

            uint64_t primary_key() const { return key.value; }
            uint64_t get_secondary_l() const { return age; }
        };

        typedef eosio::multi_index<"people"_n, person,
            indexed_by<"byage"_n, const_mem_fun<person, uint64_t, &person::get_secondary_l>>
        > address_index;
    
        /**
         * A helper method for sending a receipt to the user, every time they take an action on the contract.
         */ 
        void send_summary(name user, string message) {
            // Instantiate an Action
            action(
                // permission level - A permission_level struct
                permission_level{ get_self(), "active"_n },
                // code - The contract to call (initialised using eosio::name type)
                // "addressbook"_n would also work here, but if this contract were deployed under a different account name, it wouldn't work.
                get_self(),
                // action - The action (initialised using eosio::name type)
                // The notify action was previously defined to be called from this inline action.
                "notify"_n, 
                // data - The data to pass to the action, a tuple of positionals that correlate to the actions being called.
                std::make_tuple(user, name{user}.to_string() + message)
            ).send();
        }

        /**
         * A helper method to call count action of abcounter
         */ 
        void increment_counter(name user, string type) {
            abcounter::count_action count(
                "abcounter"_n,              // Callee contract name
                { get_self(), "active"_n }  // Permission struct 
            );
            count.send(user, type);         // Call abcontract's count action
        }
};
