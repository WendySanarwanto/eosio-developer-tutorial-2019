// #include <ctime>
#include <eosio/eosio.hpp>

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
};
