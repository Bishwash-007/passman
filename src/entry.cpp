#include "entry.hpp"

#include <stdexcept>

namespace entry
{

    namespace
    {

        void writeUint32(
            std::vector<std::uint8_t> &output,
            std::uint32_t value)
        {
            output.push_back(
                static_cast<std::uint8_t>(value & 0xFF));

            output.push_back(
                static_cast<std::uint8_t>((value >> 8) & 0xFF));

            output.push_back(
                static_cast<std::uint8_t>((value >> 16) & 0xFF));

            output.push_back(
                static_cast<std::uint8_t>((value >> 24) & 0xFF));
        }

        std::uint32_t readUint32(
            const std::vector<std::uint8_t> &data,
            std::size_t &offset)
        {
            if (offset + 4 > data.size())
            {
                throw std::runtime_error(
                    "Corrupted vault data");
            }

            std::uint32_t value =
                static_cast<std::uint32_t>(data[offset]) | (static_cast<std::uint32_t>(data[offset + 1]) << 8) | (static_cast<std::uint32_t>(data[offset + 2]) << 16) | (static_cast<std::uint32_t>(data[offset + 3]) << 24);

            offset += 4;

            return value;
        }

        void writeString(
            std::vector<std::uint8_t> &output,
            const std::string &value)
        {
            if (value.size() > UINT32_MAX)
            {
                throw std::runtime_error(
                    "String too large");
            }

            writeUint32(
                output,
                static_cast<std::uint32_t>(value.size()));

            output.insert(
                output.end(),
                value.begin(),
                value.end());
        }

        std::string readString(
            const std::vector<std::uint8_t> &data,
            std::size_t &offset)
        {
            const std::uint32_t length =
                readUint32(data, offset);

            if (
                length >
                data.size() - offset)
            {
                throw std::runtime_error(
                    "Corrupted vault data");
            }

            std::string value(
                reinterpret_cast<const char *>(
                    data.data() + offset),
                length);

            offset += length;

            return value;
        }

    }

    std::vector<std::uint8_t> serialize(
        const std::vector<Entry> &entries)
    {
        if (entries.size() > UINT32_MAX)
        {
            throw std::runtime_error(
                "Too many vault entries");
        }

        std::vector<std::uint8_t> output;

        writeUint32(
            output,
            static_cast<std::uint32_t>(entries.size()));

        for (const Entry &e : entries)
        {
            writeString(output, e.site);
            writeString(output, e.username);
            writeString(output, e.password);
            writeString(output, e.notes);
        }

        return output;
    }

    std::vector<Entry> deserialize(
        const std::vector<std::uint8_t> &data)
    {
        std::size_t offset = 0;

        const std::uint32_t count =
            readUint32(data, offset);

        constexpr std::size_t minimumEntryBytes = 16;
        if (count > (data.size() - offset) / minimumEntryBytes)
        {
            throw std::runtime_error(
                "Corrupted vault data");
        }

        std::vector<Entry> entries;
        entries.reserve(count);

        for (std::uint32_t i = 0; i < count; ++i)
        {
            Entry e;

            e.site = readString(data, offset);
            e.username = readString(data, offset);
            e.password = readString(data, offset);
            e.notes = readString(data, offset);

            entries.push_back(std::move(e));
        }

        if (offset != data.size())
        {
            throw std::runtime_error(
                "Unexpected data after vault entries");
        }

        return entries;
    }

}