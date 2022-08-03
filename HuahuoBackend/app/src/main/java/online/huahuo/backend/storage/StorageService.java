package online.huahuo.backend.storage;

import online.huahuo.backend.db.ProjectFileDB;
import org.springframework.web.multipart.MultipartFile;

import java.io.IOException;
import java.security.NoSuchAlgorithmException;

public interface StorageService {
    ProjectFileDB store(String path, MultipartFile file) throws IOException, NoSuchAlgorithmException;
    ProjectFileDB getById(Long projectId);
}
