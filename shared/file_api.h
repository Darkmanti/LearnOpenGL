#include <Windows.h>

void* Win32ReadFileToBuffer(const wchar_t* path, unsigned int* fileSize, void* handle, bool keep)
{
	DWORD read = 0;
	LARGE_INTEGER size = {};
	HANDLE file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);

	if (file == INVALID_HANDLE_VALUE)
	{
		wprintf(L"Cannot open file %s", path);
		return NULL;
	}

	void* buffer = NULL;

	if (GetFileSizeEx(file, &size) == 0)
	{
		wprintf(L"Size of file is zero %s", path);
		CloseHandle(file);
		return NULL;
	}

	size.QuadPart += 1;

	buffer = malloc(size.QuadPart);
	memset(buffer, 0, size.QuadPart);

	if (ReadFile(file, buffer, (DWORD)size.QuadPart, &read, NULL) == 0)
	{
		wprintf(L"Cannot read file %s", path);
		free(buffer);
		CloseHandle(file);
		buffer = NULL;
		return NULL;
	}

	if (fileSize != NULL)
	{
		*fileSize = size.QuadPart;
	}

	if (keep)
	{
		if (handle != NULL)
		{
			handle = file;
		}
		else
		{
			wprintf(L"Cannot assign file handle %s", path);
			free(buffer);
			CloseHandle(file);
			buffer = NULL;
			return NULL;
		}
	}
	else
	{
		// Nothing
	}

	return buffer;
}

void Win32CloseFile(void* handle)
{
	CloseHandle(handle);
}