#pragma once

#include <string>
#include <algorithm>
#include <cctype>
#include <locale>
#include <eosio/eosio.hpp>

using namespace std;
using namespace eosio;
using namespace types;

namespace utility {

    // trim from left (in place)
    static inline void ltrim(string& s) {
        s.erase( s.begin(), find_if( s.begin(), s.end(), [](int ch) {
            return !isspace(ch);
        }));
    }

    // trim from right (in place)
    static inline void rtrim(string& s) {
        s.erase( find_if( s.rbegin(), s.rend(), [](int ch) {
            return !isspace(ch);
        }).base(), s.end());
    }

    // trim from both ends (in place)
    static inline string trim(string s) {
        ltrim(s);
        rtrim(s);
        return s;
    }

    transfer_action parsetransfer(raw_transfer transfer_data)
    {
      transfer_action ta;
      auto &memo = transfer_data.memo;
      size_t n1 = memo.find(':');
      size_t n2 = memo.find(':', n1 + 1);
      size_t n3 = memo.find(':', n2 + 1);
      ta.from = name(transfer_data.from);
      ta.action = memo.substr(0, n1);
      ta.id = atoi(memo.substr(n1 + 1, n2 - (n1 + 1)).c_str());
      ta.recipient = name(memo.substr(n2 + 1));
      ta.quantity = transfer_data.quantity;
      return ta;
    }

    tuple<uint64_t, name> parsememo(const string& memo) {
        auto comma_pos = memo.find(',');
        string errormsg = "malformed memo: must have batch_id,to_account";
        check ( comma_pos != string::npos, errormsg.c_str() );
        if ( comma_pos != string::npos ) {
            check( ( comma_pos != memo.size() - 1 ), errormsg.c_str() );
        }
        // will abort if stoull throws error since wasm no error checking
        uint64_t batch_id = stoull( trim( memo.substr( 0, comma_pos ) ) );
        name to_account = name( trim ( memo.substr( comma_pos + 1 ) ) );

        return make_tuple(batch_id, to_account);
    }
}