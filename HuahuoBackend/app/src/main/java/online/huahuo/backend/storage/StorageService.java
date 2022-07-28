package online.huahuo.backend.storage;

import online.huahuo.backend.db.FileDB;
import org.springframework.core.io.Resource;
import org.springframework.web.multipart.MultipartFile;

import java.io.IOException;
import java.nio.file.Path;
import java.util.stream.Stream;

public interface StorageService {
    FileDB store(MultipartFile file) throws IOException;
    FileDB getById(Long projectId);
}
