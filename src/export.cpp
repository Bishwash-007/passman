#include "export.hpp"

#include <array>
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace export_format
{
    namespace
    {
        constexpr std::array<std::uint8_t, 4> MAGIC = {'P', 'M', 'E', '1'};

        void writeUint32(std::ofstream &file, std::uint32_t value)
        {
            for (unsigned int shift = 0; shift < 32; shift += 8)
            {
                file.put(static_cast<char>((value >> shift) & 0xFFU));
            }
        }

        std::uint32_t readUint32(const std::vector<std::uint8_t> &data,
                                 std::size_t &offset)
        {
            if (offset + 4 > data.size())
            {
                throw std::runtime_error("Malformed export file");
            }
            std::uint32_t value = 0;
            for (unsigned int shift = 0; shift < 32; shift += 8)
            {
                value |= static_cast<std::uint32_t>(data[offset++]) << shift;
            }
            return value;
        }

        void writeField(std::ofstream &file, const std::string &value)
        {
            if (value.size() > UINT32_MAX)
            {
                throw std::runtime_error("Export field is too large");
            }
            writeUint32(file, static_cast<std::uint32_t>(value.size()));
            file.write(value.data(), static_cast<std::streamsize>(value.size()));
        }

        std::string readField(const std::vector<std::uint8_t> &data,
                              std::size_t &offset)
        {
            const std::uint32_t length = readUint32(data, offset);
            if (length > data.size() - offset)
            {
                throw std::runtime_error("Malformed export file");
            }
            std::string value(
                reinterpret_cast<const char *>(data.data() + offset), length);
            offset += length;
            return value;
        }
    }

    void write(const std::string &path,
               const std::vector<entry::Entry> &entries)
    {
        if (std::filesystem::exists(path))
        {
            throw std::runtime_error("Export destination already exists: " + path);
        }
        const std::string temporaryPath = path + ".tmp";
        std::ofstream file(temporaryPath, std::ios::binary | std::ios::trunc);
        if (!file)
        {
            throw std::runtime_error("Could not write export file: " + path);
        }
        file.write(reinterpret_cast<const char *>(MAGIC.data()),
                   static_cast<std::streamsize>(MAGIC.size()));
        if (entries.size() > UINT32_MAX)
        {
            throw std::runtime_error("Too many entries to export");
        }
        writeUint32(file, static_cast<std::uint32_t>(entries.size()));
        for (const entry::Entry &storedEntry : entries)
        {
            writeField(file, storedEntry.site);
            writeField(file, storedEntry.username);
            writeField(file, storedEntry.password);
            writeField(file, storedEntry.notes);
        }
        file.close();
        if (!file)
        {
            std::filesystem::remove(temporaryPath);
            throw std::runtime_error("Failed while writing export file");
        }
        std::error_code error;
        std::filesystem::rename(temporaryPath, path, error);
        if (error)
        {
            std::filesystem::remove(temporaryPath);
            throw std::runtime_error("Could not create export file: "
                                     + error.message());
        }
    }

    std::vector<entry::Entry> read(const std::string &path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file)
        {
            throw std::runtime_error("Could not open export file: " + path);
        }
        file.seekg(0, std::ios::end);
        const std::streamsize size = file.tellg();
        if (size < 0)
        {
            throw std::runtime_error("Could not determine export file size");
        }
        file.seekg(0, std::ios::beg);
        std::vector<std::uint8_t> data(static_cast<std::size_t>(size));
        if (!data.empty())
        {
            file.read(reinterpret_cast<char *>(data.data()), size);
        }
        if (!file || data.size() < MAGIC.size() + 4
            || !std::equal(MAGIC.begin(), MAGIC.end(), data.begin()))
        {
            throw std::runtime_error("Malformed export file");
        }
        std::size_t offset = MAGIC.size();
        const std::uint32_t count = readUint32(data, offset);
        constexpr std::size_t minimumEntryBytes = 16;
        if (count > (data.size() - offset) / minimumEntryBytes)
        {
            throw std::runtime_error("Malformed export file");
        }
        std::vector<entry::Entry> entries;
        entries.reserve(count);
        for (std::uint32_t index = 0; index < count; ++index)
        {
            entries.push_back({
                readField(data, offset),
                readField(data, offset),
                readField(data, offset),
                readField(data, offset)});
        }
        if (offset != data.size())
        {
            throw std::runtime_error("Unexpected data after export entries");
        }
        return entries;
    }
}
