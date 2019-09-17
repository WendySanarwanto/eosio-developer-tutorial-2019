#include <eosio/eosio.hpp>

using namespace eosio;
using namespace std;

class [[eosio::contract("abcounter")]] abcounter : public contract {
    public:
        abcounter(name receiver, name code, datastream<const char*> ds): contract(receiver, code, ds){}

        [[eosio::action]]
        void count(name user, string type) {
            require_auth(name("addressbook"));
            count_index counts(get_self(), get_first_receiver().value);
            auto iterator = counts.find(user.value);

            if (iterator == counts.end()) {
                counts.emplace("addressbook"_n, [&](auto& row){
                    row.key = user;
                    row.emplaced = (type == "emplace") ? 1 : 0;
                    row.modified = (type == "modified") ? 1 : 0;
                    row.erased = (type == "erased") ? 1 : 0;
                });
            } else {
                counts.modify(iterator, "addressbook"_n, [&](auto& row){
                    if (type == "emplace") { row.emplaced += 1; }
                    if (type == "modified") { row.modified += 1; }
                    if (type == "erased") { row.erased += 1; }
                });
            }
        }

        using count_action = action_wrapper<"count"_n, &abcounter::count>;

    private:
        struct [[eosio::table]] counter {
            name key;
            uint64_t emplaced;
            uint64_t modified;
            uint64_t erased;
            uint64_t primary_key() const { return key.value; }
        };

        using count_index = eosio::multi_index<"counts"_n, counter>;
        // typedef eosio::multi_index<"counts"_n, counter> count_index;
};