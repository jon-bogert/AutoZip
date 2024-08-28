//#define _CRT_SECURE_NO_WARNINGS

#include <minizip/unzip.h>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <Windows.h>

#include "BasierSquare_Medium_otf.h"

#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>

#include "Timer.h"
#include "EntryPoint.h"
#include "AppData.h"

#ifdef _DEBUG
#define DBG_FILE_PATH "C:/Users/jonbo/Desktop/ableton_live_suite_11.3.25_64.zip";
//#define DBG_FILE_PATH "C:/Users/jonbo/Desktop/minizip-master.zip";
#endif // _DEBUG

void ErrorMsg(const std::string& msg)
{
    MessageBoxA(NULL, msg.c_str(), "AutoUnzip - Error", MB_OK | MB_ICONERROR);
}

class App
{
    const uint32_t WIN_WIDTH = 500;
    const uint32_t WIN_HEIGHT = 200;
    const float HANDLE_HEIGHT = 35.f;

    const float UPDATE_RATE = 1.f / 60.f;
    bool m_abort = false;
    size_t m_dataSoFar = 0;
    size_t m_totalData = 0;
    double m_invTotalData = 0.;
    std::string m_currentFile;
    xe::Timer m_timer;
    sf::Vector2i m_mousePos;

    const float BAR_WIDTH = 350;
    const float BAR_HEIGHT = 25;

    const float X_BOX_WIDTH = 20;
    const float X_BOX_HEIGHT = 2;

    sf::Color m_accentColor = { 228, 75, 77, 255 };
    std::unique_ptr<sf::RenderWindow> m_window = nullptr;
    sf::RectangleShape m_handleBar;
    sf::RectangleShape m_backgroundBar;
    sf::RectangleShape m_progressBar;
    sf::RectangleShape m_exitButton;
    sf::RectangleShape m_xBoxA;
    sf::RectangleShape m_xBoxB;
    sf::Text m_titleText;
    sf::Text m_headingText;
    sf::Text m_infoText;
    sf::Text m_percentText;
    sf::Font m_font;

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

    std::string PercentStr(float factor)
    {
        factor *= 100.f;
        std::string result = std::to_string(factor);
        size_t dotPos = result.find('.');
        if (dotPos == std::string::npos)
        {
            result += ".0";
        }
        else
        {
            result = result.substr(0, dotPos + 2);
        }
        
        result += "%";
        return result;
    }

    void SetupVisuals(const std::filesystem::path& path)
    {
        std::string fontPath;
        if (std::filesystem::exists(_APPDATA_ + "\\AutoUnzip\\config.txt"))
        {
            try
            {
                std::ifstream file(_APPDATA_ + "\\AutoUnzip\\config.txt");
                std::string line;
                std::string key;
                std::string data;
                while (std::getline(file, line))
                {
                    std::stringstream stream(line);
                    std::getline(stream, key, '=');
                    std::getline(stream, data, '=');
                    if (key == "font")
                    {
                        fontPath = data;
                        if (data.length() < 2 || data[1] != ':')
                            fontPath = "C:\\Windows\\Fonts\\" + fontPath;
                    }
                    if (key == "color")
                    {
                        std::stringstream colStream(data);
                        std::string cell;
                        std::getline(colStream, cell, ',');
                        m_accentColor.r = std::stoi(cell);
                        std::getline(colStream, cell, ',');
                        m_accentColor.g = std::stoi(cell);
                        std::getline(colStream, cell, ',');
                        m_accentColor.b = std::stoi(cell);
                    }
                }
            }
            catch (std::exception) {}
        }
        else
        {
            std::filesystem::create_directories(_APPDATA_ + "\\AutoUnzip");
            std::ofstream file(_APPDATA_ + "\\AutoUnzip\\config.txt");
            file << "font=SegoeUI.ttf\ncolor=228,75,77\n";
        }

        m_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(WIN_WIDTH, WIN_HEIGHT), "AutoUnzip", sf::Style::None);
        if (fontPath.empty() || !m_font.loadFromFile(fontPath))
        {
            if (!m_font.loadFromFile("C:/Windows/Fonts/SegoeUI.ttf"))
            {
                ErrorMsg("System font 'SegoeUI' could not be found");
                m_abort = true;
                return;
            }
        }

        m_titleText.setFont(m_font);
        m_titleText.setCharacterSize(18);
        m_titleText.setFillColor(sf::Color::White);
        m_titleText.setString("AutoUnzip");
        m_titleText.setPosition({ 10.f, 5.f });

        m_headingText.setFont(m_font);
        m_headingText.setCharacterSize(16);
        m_headingText.setFillColor(sf::Color::White);
        m_headingText.setString("Extracting: " + path.filename().u8string());
        m_headingText.setPosition({ 20.f, HANDLE_HEIGHT + 10.f });

        m_infoText.setFont(m_font);
        m_infoText.setCharacterSize(14);
        m_infoText.setFillColor({200, 200, 200, 255});
        m_infoText.setPosition({ 20.f, HANDLE_HEIGHT + 25.f + m_headingText.getLocalBounds().height });

        m_percentText.setFont(m_font);
        m_percentText.setCharacterSize(18);
        m_percentText.setFillColor(sf::Color::White);
        m_percentText.setString("0.0%");
        m_percentText.setPosition({ 20.f + BAR_WIDTH + 20.f, 150.f });

        m_handleBar.setSize({ static_cast<float>(WIN_WIDTH), HANDLE_HEIGHT });
        m_handleBar.setFillColor({ 20, 20, 20, 255 });

        m_exitButton.setSize({ HANDLE_HEIGHT, HANDLE_HEIGHT });
        m_exitButton.setPosition({ WIN_WIDTH - HANDLE_HEIGHT, 0.f });
        m_exitButton.setFillColor(sf::Color::Red);

        m_xBoxA.setSize({ X_BOX_WIDTH, X_BOX_HEIGHT });
        m_xBoxA.setFillColor(sf::Color::White);
        m_xBoxA.setOrigin({ X_BOX_WIDTH * 0.5f, X_BOX_HEIGHT * 0.5f });
        m_xBoxA.setPosition({ WIN_WIDTH - (HANDLE_HEIGHT * 0.5f), HANDLE_HEIGHT * 0.5f });
        m_xBoxA.setRotation(45.f);

        m_xBoxB.setSize({ X_BOX_WIDTH, X_BOX_HEIGHT });
        m_xBoxB.setFillColor(sf::Color::White);
        m_xBoxB.setOrigin({ X_BOX_WIDTH * 0.5f, X_BOX_HEIGHT * 0.5f });
        m_xBoxB.setPosition({ WIN_WIDTH - (HANDLE_HEIGHT * 0.5f), HANDLE_HEIGHT * 0.5f });
        m_xBoxB.setRotation(-45.f);

        m_backgroundBar.setSize({ BAR_WIDTH, BAR_HEIGHT });
        m_backgroundBar.setPosition({ 20.f, 150.f });
        m_backgroundBar.setFillColor({ 20, 20, 20, 255 });

        m_progressBar.setSize({ BAR_WIDTH * 0.25f, BAR_HEIGHT });
        m_progressBar.setPosition({ 20.f, 150.f });
        m_progressBar.setFillColor(m_accentColor);
    }

    void DrawProgress(bool drawOverride = false)
    {
        sf::Event event;
        while (m_window->pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                m_window->close();
            if (m_window->hasFocus()
                && event.type == sf::Event::MouseButtonPressed
                && event.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2i pos = sf::Mouse::getPosition(*m_window);
                if (m_exitButton.getGlobalBounds().contains({ (float)pos.x, (float)pos.y }))
                {
                    m_window->close();
                    return;
                }
            }
        }

        if (m_timer.GetElapsed() < UPDATE_RATE && !drawOverride)
            return;

        m_timer.Reset();

        double factor = m_dataSoFar * m_invTotalData;
        double percent = static_cast<int>(factor * 1000) * 0.1;

        std::string info = "Current File: " + m_currentFile + "\n"
            + DataStr(m_dataSoFar) + " / " + DataStr(m_totalData);

        m_infoText.setString(info);
        m_progressBar.setSize({ static_cast<float>(BAR_WIDTH * factor), m_progressBar.getSize().y });
        m_percentText.setString(PercentStr(factor));

        sf::Vector2i pos = sf::Mouse::getPosition(*m_window);
        if (m_window->hasFocus() 
            && m_exitButton.getGlobalBounds().contains({ (float)pos.x, (float)pos.y }))
        {
            m_exitButton.setFillColor(sf::Color::Red);
        }
        else
        {
            m_exitButton.setFillColor(m_accentColor);
        }

        m_window->clear({10 ,10, 10, 255});
        m_window->draw(m_headingText);
        m_window->draw(m_infoText);
        m_window->draw(m_backgroundBar);
        m_window->draw(m_progressBar);
        m_window->draw(m_percentText);
        m_window->draw(m_handleBar);
        m_window->draw(m_exitButton);
        m_window->draw(m_xBoxA);
        m_window->draw(m_xBoxB);
        m_window->draw(m_titleText);
        m_window->display();
    }

    void DrawAborting()
    {
        m_window->clear();

        m_window->display();
    }

    bool IsSingleRootFolderInZip(const std::filesystem::path& zipPath, std::filesystem::path& out_extractPath)
    {
        std::string newRootName;
        unzFile zipFile = unzOpen(zipPath.generic_string().c_str());
        if (!zipFile)
        {
            ErrorMsg("Failed to open zip file to gather info: " + zipPath.u8string());
            m_abort = true;
            return false;
        }

        int rootFolderCount = 0;
        int rootFileCount = 0;

        if (unzGoToFirstFile(zipFile) != UNZ_OK)
        {
            ErrorMsg("Failed to go to first file in zip");
            unzClose(zipFile);
            m_abort = true;
            return false;
        }
        do
        {
            char filename[256];
            unz_file_info fileInfo;
            if (unzGetCurrentFileInfo(zipFile, &fileInfo, filename, sizeof(filename), nullptr, 0, nullptr, 0) != UNZ_OK)
            {
                ErrorMsg("Failed to get file info: " + std::string(filename));
                unzClose(zipFile);
                m_abort = true;
                return false;
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
                m_totalData += fileInfo.uncompressed_size;
            }

        } while (unzGoToNextFile(zipFile) == UNZ_OK);

        unzClose(zipFile);

        m_invTotalData = 1. / m_totalData;

        if (rootFolderCount == 1 && rootFileCount == 0)
        {
            out_extractPath = zipPath.parent_path() / newRootName;
            return true;
        }
        return false;
    }

    void ExtractZip(const std::string& zipPath, const std::string& extractPath, bool newRoot = false)
    {
        if (std::filesystem::exists(extractPath))
        {
            std::wstring msg = L"Destination \"" + std::filesystem::path(extractPath).wstring() + L"\" already exists. Would you like to overwrite it?";
            int mbResult = MessageBox(NULL, msg.c_str(), L"AutoUnzip - Overwrite Destination?", MB_OKCANCEL | MB_ICONQUESTION);

            if (mbResult == IDOK)
            {
                std::filesystem::remove_all(extractPath);
            }
            else
            {
                return;
            }
        }

        unzFile zipFile = unzOpen(zipPath.c_str());
        if (!zipFile)
        {
            ErrorMsg("Failed to open zip file to extract: " + zipPath);
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
            ErrorMsg("Failed to go to first file in zip");
            unzClose(zipFile);
            return;
        }

        DrawProgress(true);

        do {
            char filename[256];
            unz_file_info fileInfo;
            if (unzGetCurrentFileInfo(zipFile, &fileInfo, filename, sizeof(filename), nullptr, 0, nullptr, 0) != UNZ_OK)
            {
                ErrorMsg("Failed to get file info");
                unzClose(zipFile);
                return;
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
                    ErrorMsg("Failed to open file in zip: " + std::string(filename));
                    unzClose(zipFile);
                    return;
                }

                m_currentFile = std::filesystem::path(fullPath).filename().u8string();

                std::ofstream outFile(fullPath, std::ios::binary);
                char buffer[4096];
                int bytesRead = 0;
                while ((bytesRead = unzReadCurrentFile(zipFile, buffer, sizeof(buffer))) > 0)
                {
                    if (!m_window->isOpen())
                    {
                        DrawAborting();
                        unzClose(zipFile);
                        outFile.close();
                        std::filesystem::remove(fullPath);
                        return;
                    }
                    outFile.write(buffer, bytesRead);
                    m_dataSoFar += bytesRead;
                    DrawProgress();
                }

                unzCloseCurrentFile(zipFile);
            }
        } while (unzGoToNextFile(zipFile) == UNZ_OK);

        unzClose(zipFile);
        DrawProgress(true);
    }

public:
    int Run(const std::filesystem::path& zipPath)
    {
        std::string extractPath = (zipPath.parent_path() / zipPath.stem()).u8string();
        std::filesystem::path newPath;

        SetupVisuals(zipPath);
        if (m_abort)
        {
            m_window->close();
            m_window = nullptr;
            return 1;
        }

        bool singleRootFolder = IsSingleRootFolderInZip(zipPath, newPath);
        if (m_abort)
        {
            m_window->close();
            m_window = nullptr;
            return 1;
        }

        if (singleRootFolder)
        {
            ExtractZip(zipPath.u8string(), newPath.u8string(), true);
        }
        else
        {
            ExtractZip(zipPath.u8string(), extractPath);
        }

        m_window->close();
        m_window = nullptr;

        return 0;
    }
};

int Entry(std::vector<std::string> args)
{
#ifdef _DEBUG
    std::filesystem::path zipPath = DBG_FILE_PATH;
#else // _DEBUG
    if (args.size() < 1)
    {
        ErrorMsg("No ZIP file provided.");
        return 1;
    }
    if (args[0].front() == '\"' && args[0].back() == '\"')
    {
        args[0] = args[0].substr(1, args[0].length() - 2);
    }
    std::filesystem::path zipPath = args[0];
#endif // else _DEBUG

    App app;
    return app.Run(zipPath);
}

SetEntryPoint(Entry);
