#include <iostream>
#include <string>
#include <filesystem>
#include <thread>
#include <vector>
#include <fstream>
#include <atomic>
#include <mutex>

// Headers
#include "headers/infostream.h"
#include <ShObjIdl_core.h>
#include <Windows.h>

// Zip example password = "1234"

long long linesInFile(const std::wstring& path) {
    std::wifstream file(path);
    if (!file.is_open()) return 0;
    long long lineCount = 0;
    std::wstring line;
    while (std::getline(file, line)) ++lineCount;
    file.close();
    return lineCount;
}

std::string wstringToString(const std::wstring& wstr) {
    if (wstr.empty()) return {};
    std::size_t len = wstr.size() * 4;
    std::vector<char> buffer(len);
    std::wcstombs(buffer.data(), wstr.c_str(), buffer.size());
    return std::string(buffer.data());
}

std::wstring openFileDialog() {
    std::wstring selectedFilePath;
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr)) return selectedFilePath;

    IFileOpenDialog* pFileOpen;
    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
    if (FAILED(hr)) {
        CoUninitialize();
        return selectedFilePath;
    }

    DWORD dwOptions;
    hr = pFileOpen->GetOptions(&dwOptions);
    if (SUCCEEDED(hr)) {
        pFileOpen->SetOptions(dwOptions | FOS_ALLOWMULTISELECT);
    }

    hr = pFileOpen->Show(NULL);
    if (SUCCEEDED(hr)) {
        IShellItemArray* pItems;
        hr = pFileOpen->GetResults(&pItems);
        if (SUCCEEDED(hr)) {
            DWORD dwNumItems;
            hr = pItems->GetCount(&dwNumItems);
            if (SUCCEEDED(hr) && dwNumItems > 0) {
                IShellItem* pItem;
                hr = pItems->GetItemAt(0, &pItem);
                if (SUCCEEDED(hr)) {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                    if (SUCCEEDED(hr)) {
                        selectedFilePath = pszFilePath;
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pItems->Release();
        }
    }
    pFileOpen->Release();
    CoUninitialize();
    return selectedFilePath;
}

bool archiveExists(const std::string& name) {
    return std::filesystem::exists(name);
}

bool testPassword(const std::string& password, const std::string& zipPath) {
    std::string command = "7z t -p" + password + " " + zipPath + " >nul 2>&1";
    return system(command.c_str()) == 0;
}

void generatePassword(const char* charset, std::string current, int maxLength, const std::string& zipPath, std::atomic<bool>& found) {
    if (found) return;
    if (current.length() == maxLength) {
        if (testPassword(current, zipPath)) {
            success(("Password found: " + current).c_str());
            found = true;
        }
        return;
    }

    for (int i = 0; charset[i] && !found; i++) {
        generatePassword(charset, current + charset[i], maxLength, zipPath, found);
    }
}

void threadedGeneratePassword(const char* charset, int start, int end, int maxLength, const std::string& zipPath, std::atomic<bool>& found) {
    for (int i = start; i < end && !found; ++i) {
        std::string initial(1, charset[i]);
        generatePassword(charset, initial, maxLength, zipPath, found);
    }
}

void bruteforce(const std::string& zipPath) {
    constexpr char charset[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int maxLength = std::stoi(userInput("Enter the maximum length of the password: "));

    int numThreads = std::thread::hardware_concurrency();
    notify(("Utilizing " + std::to_string(numThreads) + " threads for brute-force attack.").c_str());
    int charsetLength = sizeof(charset) - 1;
    int range = charsetLength / numThreads;

    std::vector<std::thread> threads;
    std::atomic<bool> found(false);

    for (int i = 0; i < numThreads; ++i) {
        int start = i * range;
        int end = (i + 1) * range;
        if (i == numThreads - 1) end = charsetLength;
        threads.emplace_back(threadedGeneratePassword, charset, start, end, maxLength, zipPath, std::ref(found));
    }

    for (auto& t : threads) {
        t.join();
    }

    if (!found) {
        alert("Password not found!");
    }
}

void processLines(long long startLine, long long numLines, const std::wstring& dictionaryPath, const std::string& zipPath, std::atomic<bool>& found) {
    std::wifstream file(dictionaryPath);
    if (!file.is_open()) {
        alert("Failed to open dictionary file!");
        return;
    }

    std::wstring line;
    for (long long i = 0; i < startLine && std::getline(file, line); ++i);

    for (long long i = 0; i < numLines && !found && std::getline(file, line); ++i) {
        if (testPassword(std::string(line.begin(), line.end()), zipPath)) {
            success(("Password found: " + std::string(line.begin(), line.end())).c_str());
            found = true;
            return;
        }
    }
}

void dictionaryMultiThreaded(const std::string& zipPath) {
    std::wstring dictionaryPath = openFileDialog();
    if (dictionaryPath.empty()) {
        alert("No dictionary file selected!");
        return;
    }

    long long totalLines = linesInFile(dictionaryPath);
    if (totalLines == 0) {
        alert("Dictionary file is empty!");
        return;
    }

    int numThreads = std::thread::hardware_concurrency();
    notify(("Utilizing " + std::to_string(numThreads) + " threads for dictionary attack.").c_str());
    std::vector<std::thread> threads;
    std::atomic<bool> found(false);

    long long linesPerThread = totalLines / numThreads;
    long long extraLines = totalLines % numThreads;
    long long currentLine = 0;

    for (int i = 0; i < numThreads; ++i) {
        long long linesToProcess = linesPerThread + (i < extraLines ? 1 : 0);
        threads.emplace_back(processLines, currentLine, linesToProcess, dictionaryPath, zipPath, std::ref(found));
        currentLine += linesToProcess;
    }

    for (auto& thread : threads) {
        thread.join();
    }

    if (!found) {
        alert("Password not found!");
    }
}





int main() {
    SetConsoleTitleW(L"7zip Password Cracker | Version 1.0 | By: @ohusq");

    notify("Welcome to the 7zip password cracker");
    alert("This program is for recovery purposes only");
    alert("Do not use this program for illegal purposes!\n");

    notify("Select your type of attack:");
    notify("1. Brute-force attack");
    notify("2. Dictionary attack");
    notify("3. Exit\n");

    std::string input = userInput("Enter a choice: ");
    int choice = std::stoi(input);

    if (choice == 3) {
        notify("Exiting program...");
        return 0;
    }

    notify("Please select the 7zip archive file.");
    std::wstring zipPath = openFileDialog();
    if (zipPath.empty()) {
        alert("No archive file selected!");
        return 1;
    }
    

    std::string zipPathStr = wstringToString(zipPath);

    if (!archiveExists(zipPathStr)) {
        alert("The archive does not exist!");
        return 1;
    }

    switch (choice) {
        case 1:
            notify("Brute-force attack selected.");
            bruteforce(zipPathStr);
            break;
        case 2:
            notify("Dictionary attack selected.");
            dictionaryMultiThreaded(zipPathStr);
            break;
        default: 
            warn("Invalid choice selected.");
            break;
    }

    return 0;
}
