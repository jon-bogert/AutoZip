#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <minizip/unzip.h>

#include "Timer.h"

const float UPDATE_RATE = 0.1f;
size_t g_dataSoFar = 0;
double g_invTotalData = 0.;
xe::Timer g_timer;

void DrawProgress(bool drawOverride = false)
{
    if (g_timer.GetElapsed() < UPDATE_RATE && !drawOverride)
        return;

    g_timer.Reset();
    double factor = g_dataSoFar * g_invTotalData;
    double percent = static_cast<int>(factor * 1000) * 0.1;

    int numBars = static_cast<int>(percent * 0.2);

    std::cout << "\r[";
    for (int i = 0; i < numBars; ++i)
    {
        std::cout << '=';
    }
    for (int i = numBars; i < 20; ++i)
    {
        std::cout << ' ';
    }
    std::cout << "] " << percent << "%      ";
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
    size_t totalSize = 0;
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
            totalSize += fileInfo.uncompressed_size;
        }

    } while (unzGoToNextFile(zipFile) == UNZ_OK);

    unzClose(zipFile);

    g_invTotalData = 1. / totalSize;

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

            std::ofstream outFile(fullPath, std::ios::binary);
            char buffer[4096];
            int bytesRead = 0;
            while ((bytesRead = unzReadCurrentFile(zipFile, buffer, sizeof(buffer))) > 0)
            {
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
    if (argc < 2)
    {
        std::cerr << "No ZIP file provided." << std::endl;
        return 1;
    }

    std::filesystem::path zipPath = argv[1];
    std::string extractPath = (zipPath.parent_path() / zipPath.stem()).u8string();
    std::filesystem::path newPath;

    if (IsSingleRootFolderInZip(zipPath, newPath))
    {
        ExtractZip(zipPath.u8string(), newPath.u8string(), true);
    }
    else
    {
        ExtractZip(zipPath.u8string(), extractPath);
    }

    return 0;
}
