/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cassert>

#include <ignite/impl/binary/binary_utils.h>

#include "ignite/odbc/utility.h"
#include "ignite/odbc/system/odbc_constants.h"

#ifdef ODBC_DEBUG

FILE* log_file = NULL;

void logInit(const char* path)
{
    if (!log_file)
    {
        log_file = fopen(path, "w");
    }
}

#endif //ODBC_DEBUG

namespace ignite
{
    namespace utility
    {
        size_t CopyStringToBuffer(const std::string& str, char* buf, size_t buflen)
        {
            if (!buf || !buflen)
                return 0;

            size_t bytesToCopy = std::min(str.size(), static_cast<size_t>(buflen - 1));

            memcpy(buf, str.data(), bytesToCopy);
            buf[bytesToCopy] = 0;

            return bytesToCopy;
        }

        void ReadString(ignite::impl::binary::BinaryReaderImpl& reader, std::string& str)
        {
            int32_t strLen = reader.ReadString(0, 0);
            if (!strLen)
            {
                str.clear();

                char dummy;

                reader.ReadString(&dummy, sizeof(dummy));
            }
            else
            {
                str.resize(strLen);

                reader.ReadString(&str[0], static_cast<int32_t>(str.size()));
            }
        }

        void WriteString(ignite::impl::binary::BinaryWriterImpl& writer, const std::string & str)
        {
            writer.WriteString(str.data(), static_cast<int32_t>(str.size()));
        }

        void ReadDecimal(ignite::impl::binary::BinaryReaderImpl& reader, Decimal& decimal)
        {
            int8_t hdr = reader.ReadInt8();

            assert(hdr == ignite::impl::binary::IGNITE_TYPE_DECIMAL);

            int32_t scale = reader.ReadInt32();

            int32_t len = reader.ReadInt32();

            std::vector<int8_t> mag;

            mag.resize(len);

            impl::binary::BinaryUtils::ReadInt8Array(reader.GetStream(), mag.data(), static_cast<int32_t>(mag.size()));

            Decimal res(scale, mag.data(), static_cast<int32_t>(mag.size()));

            swap(decimal, res);
        }

        void WriteDecimal(ignite::impl::binary::BinaryWriterImpl& writer, const Decimal& decimal)
        {
            writer.WriteInt8(ignite::impl::binary::IGNITE_TYPE_DECIMAL);

            writer.WriteInt32(decimal.GetScale() | (decimal.IsNegative() ? 0x80000000 : 0));

            writer.WriteInt32(decimal.GetLength());

            impl::binary::BinaryUtils::WriteInt8Array(writer.GetStream(), decimal.GetMagnitude(), decimal.GetLength());
        }

        std::string SqlStringToString(const unsigned char* sqlStr, int32_t sqlStrLen)
        {
            std::string res;

            const char* sqlStrC = reinterpret_cast<const char*>(sqlStr);

            if (!sqlStr || !sqlStrLen)
                return res;

            if (sqlStrLen == SQL_NTS)
                res.assign(sqlStrC);
            else
                res.assign(sqlStrC, sqlStrLen);

            return res;
        }

        time_t DateToCTime(const Date& date)
        {
            return static_cast<time_t>(date.GetSeconds());
        }

        time_t TimestampToCTime(const Timestamp& ts)
        {
            return static_cast<time_t>(ts.GetSeconds());
        }
    }
}

