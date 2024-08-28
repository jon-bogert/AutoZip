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

class App
{
    const uint32_t WIN_WIDTH = 500;
    const uint32_t WIN_HEIGHT = 250;
    const float HANDLE_HEIGHT = 25.f;

    const float UPDATE_RATE = 1.f / 60.f;
    size_t m_dataSoFar = 0;
    size_t m_totalData = 0;
    double m_invTotalData = 0.;
    std::string m_currentFile;
    xe::Timer m_timer;

    const float BAR_WIDTH = 350;
    const float BAR_HEIGHT = 25;
    std::unique_ptr<sf::RenderWindow> m_window = nullptr;
    sf::RectangleShape m_handleBar;
    sf::RectangleShape m_backgroundBar;
    sf::RectangleShape m_progressBar;
    sf::Text m_headingText;
    sf::Text m_infoText;
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

    void SetupVisuals(const std::filesystem::path& path)
    {
        m_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(WIN_WIDTH, WIN_HEIGHT), "AutoZip", sf::Style::None);
        m_font.loadFromFile("C:/Windows/Fonts/SegoeUI.ttf");

        m_headingText.setFont(m_font);
        m_headingText.setCharacterSize(18);
        m_headingText.setFillColor(sf::Color::White);
        m_headingText.setString("Extracting: " + path.filename().u8string());
        m_headingText.setPosition({ 10.f, HANDLE_HEIGHT + 10.f });

        m_infoText.setFont(m_font);
        m_infoText.setCharacterSize(14);
        m_infoText.setFillColor(sf::Color::White);
        m_infoText.setPosition({ 10.f, HANDLE_HEIGHT + 25.f + m_headingText.getLocalBounds().height });

        //m_handleBar.setScale{WINDOW}

        m_backgroundBar.setSize({ BAR_WIDTH, BAR_HEIGHT });
        m_backgroundBar.setPosition({ 25.f, 150.f });
        m_backgroundBar.setFillColor({ 20, 20, 20, 255 });

        m_progressBar.setSize({ BAR_WIDTH * 0.25f, BAR_HEIGHT });
        m_progressBar.setPosition({ 25.f, 150.f });
        m_progressBar.setFillColor(sf::Color::Red);
    }

    void DrawProgress(bool drawOverride = false)
    {
        if (m_timer.GetElapsed() < UPDATE_RATE && !drawOverride)
            return;

        m_timer.Reset();

        double factor = m_dataSoFar * m_invTotalData;
        double percent = static_cast<int>(factor * 1000) * 0.1;

        std::string info = "Current File: " + m_currentFile + "\n"
            + DataStr(m_dataSoFar) + " / " + DataStr(m_totalData);

        m_infoText.setString(info);
        m_progressBar.setSize({ static_cast<float>(BAR_WIDTH * factor), m_progressBar.getSize().y });

        sf::Event event;
        while (m_window->pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                m_window->close();
        }

        m_window->clear();
        m_window->draw(m_headingText);
        m_window->draw(m_infoText);
        m_window->draw(m_backgroundBar);
        m_window->draw(m_progressBar);
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
        if (IsSingleRootFolderInZip(zipPath, newPath))
        {
            ExtractZip(zipPath.u8string(), newPath.u8string(), true);
        }
        else
        {
            ExtractZip(zipPath.u8string(), extractPath);
        }

        m_window->close();
        m_window = nullptr;
        //g_fontData.clear();

        return 0;
    }
};

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

    App app;
    return app.Run(zipPath);
}
