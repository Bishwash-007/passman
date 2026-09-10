#pragma once

#include <cstddef>
#include <string>

#include "vault.hpp"

void handleAdd(
    vault::Vault &vault,
    const std::string &site,
    bool generatePassword);

void handleGet(
    const vault::Vault &vault,
    const std::string &site,
    bool copyPassword);

void handleList(
    const vault::Vault &vault);

void handleDelete(
    vault::Vault &vault,
    const std::string &site);

void handleSearch(
    const vault::Vault &vault,
    const std::string &query);

void handleChangeMaster(
    vault::Vault &vault);

void handleGenerate(std::size_t length);
void handleExport(const vault::Vault &vault, const std::string &path);
void handleImport(vault::Vault &vault, const std::string &path);