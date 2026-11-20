#pragma once

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace ParaEngine
{
	inline bool MoveWebFile(const std::string& src, const std::string& dest)
	{
		if (src == dest || std::rename(src.c_str(), dest.c_str()) == 0)
			return true;
		if (errno != EXDEV)
			return false;

		// /temp is on MEMFS and /worlds is on IDBFS. Stage regular files on
		// the destination mount so readers only ever see the complete file.
		struct stat info;
		if (::lstat(src.c_str(), &info) != 0 || !S_ISREG(info.st_mode))
			return false;

		std::string staging = dest.substr(0, dest.find_last_of('/') + 1) + ".npl-move-XXXXXX";
		FILE* input = std::fopen(src.c_str(), "rb");
		if (!input)
			return false;

		int fd = ::mkstemp(&staging[0]);
		if (fd < 0)
		{
			std::fclose(input);
			return false;
		}

		FILE* output = ::fdopen(fd, "wb");
		bool copied = false;
		if (output)
		{
			char buffer[8192];
			size_t count;
			while ((count = std::fread(buffer, 1, sizeof(buffer), input)) > 0)
			{
				if (std::fwrite(buffer, 1, count, output) != count)
					break;
			}
			copied = count == 0 && !std::ferror(input);
			if (std::fclose(output) != 0)
				copied = false;
		}
		else
			::close(fd);
		std::fclose(input);

		if (!copied || std::rename(staging.c_str(), dest.c_str()) != 0)
		{
			::unlink(staging.c_str());
			return false;
		}
		return ::unlink(src.c_str()) == 0;
	}
}
