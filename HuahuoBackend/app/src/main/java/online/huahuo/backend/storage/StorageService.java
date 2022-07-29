package online.huahuo.backend.storage;

import online.huahuo.backend.db.ProjectFileDB;
import org.springframework.web.multipart.MultipartFile;

import java.io.IOException;

public interface StorageService {
    ProjectFileDB store(MultipartFile file) throws IOException;
    ProjectFileDB getById(Long projectId);
}
