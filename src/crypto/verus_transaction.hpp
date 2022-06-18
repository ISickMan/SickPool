#ifndef VERUS_TRANSACTION_HPP_
#define VERUS_TRANSACTION_HPP_
#include <iomanip>
#include <iostream>
#include <tuple>
#include <vector>

#include "transaction.hpp"
#include "utils.hpp"
#include "verushash/endian.h"

class VerusTransaction : public Transaction
{
   public:
    VerusTransaction(int32_t ver, uint32_t locktime, bool overwintered,
                     uint32_t verGroupId)
        : Transaction(ver, locktime),
          is_overwintered(overwintered),
          version_groupid(verGroupId)
    {
    }

    std::vector<unsigned char> GetBytes() override
    {
        uint64_t vinVarIntVal = vin.size();
        char vinVarIntLen = VarInt(vinVarIntVal);

        uint64_t voutVarIntVal = vout.size();
        char voutVarIntLen = VarInt(voutVarIntVal);

        tx_len += 4 * 4 + 11 + vinVarIntLen + voutVarIntLen;
        // bytes.resize(tx_len);
        std::vector<unsigned char> bytes(tx_len);

        int ver = version | (is_overwintered << 31);
        WriteData(bytes.data(), &ver, 4);
        WriteData(bytes.data(), &version_groupid, 4);

        WriteData(bytes.data(), &vinVarIntVal, vinVarIntLen);

        for (Input input : this->vin)
        {
            WriteData(bytes.data(), input.previous_output.txid_hash, 32);
            WriteData(bytes.data(), &input.previous_output.index, 4);

            WriteData(bytes.data(), &input.sig_compact_val,
                      input.sig_compact_len);
            WriteData(bytes.data(), input.signature_script.data(),
                      input.signature_script.size());

            WriteData(bytes.data(), &input.sequence, 4);
        }

        WriteData(bytes.data(), &voutVarIntVal, voutVarIntLen);
        for (Output output : this->vout)
        {
            WriteData(bytes.data(), &output.value, 8);

            WriteData(bytes.data(), &output.script_compact_val,
                      output.script_compact_len);
            WriteData(bytes.data(), output.pk_script.data(),
                      output.pk_script.size());
        }

        WriteData(bytes.data(), &lock_time, 4);
        WriteData(bytes.data(), &expiryHeight, 4);

        // empty data for privacy stuff
        memset(bytes.data() + written, 0, 11);

        // for (int i = 0; i < bytes.size(); i++)
        //     std::cout << std::hex << std::setfill('0') << std::setw(2)
        //               << (int)bytes[i];
        // std::cout << std::endl;
        return bytes;
    }

    // since PBAAS_ACTIVATE
    void AddFeePoolOutput(std::string_view coinbaseHex)
    {
        uint8_t coinbaseBin[coinbaseHex.size() / 2];
        Unhexlify(coinbaseBin, coinbaseHex.data(), coinbaseHex.size());

        // skip header and versiongroupid
        uint8_t* pos = coinbaseBin + 4 + 4;
        std::pair<uint64_t, uint8_t> inputCountVI = ReadNumScript(pos);
        pos += inputCountVI.second;

        for (int i = 0; i < inputCountVI.first; i++)
        {
            pos += sizeof(OutPoint); // skip outpoint
            auto inputVI = ReadNumScript(pos);
            pos += inputVI.second + inputVI.first;
            pos += 4; // sequence
        }

        auto outputCountVI = ReadNumScript(pos);
        pos += outputCountVI.second;
        // keep the last output (fee pool)
        for (int i = 0; i < outputCountVI.first - 1; i++)
        {
            pos += sizeof(int64_t); // skip amount value
            auto outputVI = ReadNumScript(pos);
            pos += outputVI.second + outputVI.first;
        }

        pos += sizeof(int64_t);  // skip amount value
        auto feePoolVI = ReadNumScript(pos);
        pos += feePoolVI.second;

        Output output;
        output.value = 0;
        output.pk_script = std::vector<uint8_t>(pos, pos + feePoolVI.first);

        uint64_t varIntVal = output.pk_script.size();
        char varIntLen = VarInt(varIntVal);
        output.script_compact_val = varIntVal;
        output.script_compact_len = varIntLen;

        vout.push_back(output);
        tx_len += sizeof(output.value) + output.pk_script.size() + varIntLen;
    }

   private:
    template <typename T>
    void WriteData(unsigned char* bytes, T* val, int size)
    {
        memcpy(bytes + written, val, size);
        written += size;
    }

    int written = 0;
    const bool is_overwintered;
    const uint32_t version_groupid;
    const uint32_t expiryHeight = 0;
};
#endif