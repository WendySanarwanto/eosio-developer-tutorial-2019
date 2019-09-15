// #include <ctime>
#include <eosio/eosio.hpp>

using namespace eosio;
using namespace std;

class [[eosio:contract]] addressbook: public contract {
    public:   
        // using contract::contract;
        employees(name receiver, name code, datastream<const char*> ds): contract(receiver, code, ds) {}

        void upsert(
            name user,
            string first_name,
            string last_name,
            string street,
            string city,
            string state
        ) {
            // only the user has control over their own record
            require_auth(user);
            // Instantiate address book table:
            // - 1st param: get_self() function which will pass the name of this contract.
            // - 2nd param: get_first_receiver is the account name this contract is deployed to
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
                    row.street = street;
                    row.city = city;
                    row.state = state;
                });
            } else {
                // The user exist in the table
                // update existing record by using the multi_index's "modify" method.
                addresses.modify(iterator, user, [&]( auto& row ){
                    row.key = user;
                    row.first_name = first_name;
                    row.last_name = last_name;
                    row.street = street;
                    row.city = city;
                    row.state = state;                    
                });
            }
        }

        void erase(name user) {
            // only the user has control over their own record
            require_auth(user);
            address_index addresses(get_self(), get_first_receiver().value);
            auto iterator = addresses.find(user.value);
            check(iterator != addresses.end(), "Record does not exist.");
            addresses.erase(iterator);
        }

    private:
        struct person {
            name key;
            string first_name;
            string last_name;
            string street;
            string city;
            string state;

            uint64_t primary_key() const { return key.value; }
        };

        typedef eosio::multi_index<"people"_n, person> address_index;
};
