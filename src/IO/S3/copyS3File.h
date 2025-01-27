#pragma once

#include "config.h"

#if USE_AWS_S3

#include <Storages/StorageS3Settings.h>
#include <Interpreters/threadPoolCallbackRunner.h>
#include <base/types.h>
#include <functional>
#include <memory>


namespace DB
{
class SeekableReadBuffer;

using CreateReadBuffer = std::function<std::unique_ptr<SeekableReadBuffer>()>;

/// Copies a file from S3 to S3.
/// The same functionality can be done by using the function copyData() and the classes ReadBufferFromS3 and WriteBufferFromS3
/// however copyS3File() is faster and spends less network traffic and memory.
/// The parameters `src_offset` and `src_size` specify a part in the source to copy.
///
/// Note, that it tries to copy file using native copy (CopyObject), but if it
/// has been disabled (with settings.allow_native_copy) or request failed
/// because it is a known issue, it is fallbacks to read-write copy
/// (copyDataToS3File()).
///
/// s3_client_with_long_timeout (may be equal to s3_client) is used for native copy and
/// CompleteMultipartUpload requests. These requests need longer timeout because S3 servers often
/// block on them for multiple seconds without sending or receiving data from us (maybe the servers
/// are copying data internally, or maybe throttling, idk).
void copyS3File(
    const std::shared_ptr<const S3::Client> & s3_client,
    const std::shared_ptr<const S3::Client> & s3_client_with_long_timeout,
    const String & src_bucket,
    const String & src_key,
    size_t src_offset,
    size_t src_size,
    const String & dest_bucket,
    const String & dest_key,
    const S3Settings::RequestSettings & settings,
    const std::optional<std::map<String, String>> & object_metadata = std::nullopt,
    ThreadPoolCallbackRunner<void> schedule_ = {},
    bool for_disk_s3 = false);

/// Copies data from any seekable source to S3.
/// The same functionality can be done by using the function copyData() and the class WriteBufferFromS3
/// however copyDataToS3File() is faster and spends less memory.
/// The callback `create_read_buffer` can be called from multiple threads in parallel, so that should be thread-safe.
/// The parameters `offset` and `size` specify a part in the source to copy.
void copyDataToS3File(
    const CreateReadBuffer & create_read_buffer,
    size_t offset,
    size_t size,
    const std::shared_ptr<const S3::Client> & dest_s3_client,
    const std::shared_ptr<const S3::Client> & dest_s3_client_with_long_timeout,
    const String & dest_bucket,
    const String & dest_key,
    const S3Settings::RequestSettings & settings,
    const std::optional<std::map<String, String>> & object_metadata = std::nullopt,
    ThreadPoolCallbackRunner<void> schedule_ = {},
    bool for_disk_s3 = false);

}

#endif
