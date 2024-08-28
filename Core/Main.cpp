//#define _CRT_SECURE_NO_WARNINGS

#include <minizip/unzip.h>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include "BasierSquare_Medium_otf.h"

#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>

#include "Timer.h"

#ifdef _DEBUG
#define DBG_FILE_PATH "C:/Users/jonbo/Desktop/ableton_live_suite_11.3.25_64.zip";
//#define DBG_FILE_PATH "C:/Users/jonbo/Desktop/minizip-master.zip";
#endif // _DEBUG

const uint32_t WIN_WIDTH = 500;
const uint32_t WIN_HEIGHT = 250;

const float UPDATE_RATE = 1.f / 60.f;
size_t g_dataSoFar = 0;
size_t g_totalData = 0;
double g_invTotalData = 0.;
std::string g_currentFile;
xe::Timer g_timer;

const uint32_t BAR_WIDTH = 350;
const uint32_t BAR_HEIGHT = 25;
std::unique_ptr<sf::RenderWindow> g_window = nullptr;
sf::RectangleShape g_backgroundBar;
sf::RectangleShape g_progressBar;
sf::Text g_headingText;
sf::Text g_infoText;
//std::vector<uint8_t> g_fontData;
sf::Font g_font;

std::string DataStr(const size_t size)
{
    std::string result;
    if (size > 1000000000)
    {
        float gb = size * 0.000000001f;
        result = std::to_string(gb);
        size_t dotPos = result.find('.');
        if (dotPos == std::string::npos)
        {
            result += ".0";
        }
        else
        {
            result = result.substr(0, dotPos + 2);
        }
        result += " GB";
    }
    else if (size > 1000000)
    {
        float mb = size * 0.000001f;
        result = std::to_string(mb);
        size_t dotPos = result.find('.');
        if (dotPos == std::string::npos)
        {
            result += ".0";
        }
        else
        {
            result = result.substr(0, dotPos + 2);
        }
        result += " MB";
    }
    else if (size > 1000)
    {
        float kb = size * 0.001f;
        result = std::to_string(kb);
        size_t dotPos = result.find('.');
        if (dotPos == std::string::npos)
        {
            result += ".0";
        }
        else
        {
            result = result.substr(0, dotPos + 2);
        }
        result += " KB";
    }
    else
    {
        result = std::to_string(size) + " bytes";
    }
    return result;
}

void SetupVisuals(const std::filesystem::path& path)
{
    g_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(WIN_WIDTH, WIN_HEIGHT), "AutoZip", sf::Style::None);
    //res::BasierSquare_Medium_otf(g_fontData);
    //g_font.loadFromMemory(g_fontData.data(), g_fontData.size());
    g_font.loadFromFile("C:/Windows/Fonts/SegoeUI.ttf");

    g_headingText.setFont(g_font);
    g_headingText.setCharacterSize(20);
    g_headingText.setFillColor(sf::Color::White);
    g_headingText.setString("Extracting: " + path.filename().u8string());
    g_headingText.setPosition({ 10.f, 10.f });

    g_infoText.setFont(g_font);
    g_infoText.setCharacterSize(8);
    g_infoText.setFillColor(sf::Color::White);
    g_infoText.setPosition({ 10.f, 10.f + g_headingText.getLocalBounds().height});
    

    g_backgroundBar.setSize({ BAR_WIDTH, BAR_HEIGHT });
    g_backgroundBar.setPosition({ 25.f, 150.f });
    g_backgroundBar.setFillColor({ 20, 20, 20, 255 });

    g_progressBar.setSize({ BAR_WIDTH * 0.25f, BAR_HEIGHT });
    g_progressBar.setPosition({ 25.f, 150.f });
    g_progressBar.setFillColor(sf::Color::Red);
}

void DrawProgress(bool drawOverride = false)
{
    if (g_timer.GetElapsed() < UPDATE_RATE && !drawOverride)
        return;

    g_timer.Reset();

    double factor = g_dataSoFar * g_invTotalData;
    double percent = static_cast<int>(factor * 1000) * 0.1;

    std::string info = "Current File: " + g_currentFile + "\n"
        + DataStr(g_dataSoFar) + " / " + DataStr(g_totalData);

    g_infoText.setString(info);
    g_progressBar.setSize({ static_cast<float>(BAR_WIDTH * factor), g_progressBar.getSize().y });

    sf::Event event;
    while (g_window->pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            g_window->close();
    }

    g_window->clear();
    g_window->draw(g_headingText);
    g_window->draw(g_infoText);
    g_window->draw(g_backgroundBar);
    g_window->draw(g_progressBar);
    g_window->display();
}

void DrawAborting()
{
    g_window->clear();

    g_window->display();
}

bool IsSingleRootFolderInZip(const std::filesystem::path& zipPath, std::filesystem::path& out_extractPath)
{
    std::string newRootName;
    unzFile zipFile = unzOpen(zipPath.u8string().c_str());
    if (!zipFile)
    {
        std::cerr << "Failed to open zip file: " << zipPath << std::endl;
        return false;
    }

    int rootFolderCount = 0;
    int rootFileCount = 0;

    if (unzGoToFirstFile(zipFile) != UNZ_OK)
    {
        std::cerr << "Failed to go to first file in zip" << std::endl;
        unzClose(zipFile);
        return false;
    }
    do
    {
        char filename[256];
        unz_file_info fileInfo;
        if (unzGetCurrentFileInfo(zipFile, &fileInfo, filename, sizeof(filename), nullptr, 0, nullptr, 0) != UNZ_OK)
        {
            std::cerr << "Failed to get file info" << std::endl;
            break;
        }

        std::string filePath(filename);
        if (filePath.find('/') == std::string::npos)
        {
            rootFileCount++;
        }
        else if (filePath.find('/') == filePath.length() - 1)
        {
            rootFolderCount++;
            newRootName = filePath.substr(0, filePath.length() - 1);
        }

        if (filePath.back() != '/')
        {
            g_totalData += fileInfo.uncompressed_size;
        }

    } while (unzGoToNextFile(zipFile) == UNZ_OK);

    unzClose(zipFile);

    g_invTotalData = 1. / g_totalData;

    if (rootFolderCount == 1 && rootFileCount == 0)
    {
        out_extractPath = zipPath.parent_path() / newRootName;
        return true;
    }
    return false;
}

void ExtractZip(const std::string& zipPath, const std::string& extractPath, bool newRoot = false)
{
    unzFile zipFile = unzOpen(zipPath.c_str());
    if (!zipFile)
    {
        std::cerr << "Failed to open zip file: " << zipPath << std::endl;
        return;
    }

    std::filesystem::create_directories(extractPath);
    size_t rootLength = 0;
    if (newRoot)
    {
        rootLength = std::filesystem::path(extractPath).filename().string().length() + 1;
    }

    if (unzGoToFirstFile(zipFile) != UNZ_OK)
    {
        std::cerr << "Failed to go to first file in zip" << std::endl;
        unzClose(zipFile);
        return;
    }

    DrawProgress(true);

    do {
        char filename[256];
        unz_file_info fileInfo;
        if (unzGetCurrentFileInfo(zipFile, &fileInfo, filename, sizeof(filename), nullptr, 0, nullptr, 0) != UNZ_OK)
        {
            std::cerr << "Failed to get file info" << std::endl;
            break;
        }
        std::string filePath = (newRoot) ?
            std::string(filename).substr(rootLength) :
            filename;

        std::string fullPath = extractPath + "/" + filePath;
        if (filePath.empty())
        {
            continue;
        }
        if (filePath.back() == '/')
        {
            std::filesystem::create_directories(fullPath);
        }
        else
        {
            if (unzOpenCurrentFile(zipFile) != UNZ_OK)
            {
                std::cerr << "Failed to open file in zip: " << filename << std::endl;
                continue;
            }

            g_currentFile = std::filesystem::path(fullPath).filename().u8string();

            std::ofstream outFile(fullPath, std::ios::binary);
            char buffer[4096];
            int bytesRead = 0;
            while ((bytesRead = unzReadCurrentFile(zipFile, buffer, sizeof(buffer))) > 0)
            {
                if (!g_window->isOpen())
                {
                    DrawAborting();
                    unzClose(zipFile);
                    outFile.close();
                    std::filesystem::remove(fullPath);
                    return;
                }
                outFile.write(buffer, bytesRead);
                g_dataSoFar += bytesRead;
                DrawProgress();
            }

            unzCloseCurrentFile(zipFile);
        }
    } while (unzGoToNextFile(zipFile) == UNZ_OK);

    unzClose(zipFile);
    DrawProgress(true);
}

int main(int argc, char* argv[])
{
#ifdef _DEBUG
    std::filesystem::path zipPath = DBG_FILE_PATH;
#else // _DEBUG
    if (argc < 2)
    {
        std::cerr << "No ZIP file provided." << std::endl;
        return 1;
    }

    std::filesystem::path zipPath = argv[1];
#endif // else _DEBUG
    std::string extractPath = (zipPath.parent_path() / zipPath.stem()).u8string();
    std::filesystem::path newPath;

    SetupVisuals(zipPath);
    if (IsSingleRootFolderInZip(zipPath, newPath))
    {
        ExtractZip(zipPath.u8string(), newPath.u8string(), true);
    }
    else
    {
        ExtractZip(zipPath.u8string(), extractPath);
    }

    g_window->close();
    g_window = nullptr;
    //g_fontData.clear();

    return 0;
}
