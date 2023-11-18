package online.huahuo.backend.storage;

import online.huahuo.backend.db.BinaryFileDB;
import org.springframework.web.multipart.MultipartFile;

import java.io.IOException;
import java.security.NoSuchAlgorithmException;

public interface StorageService {
    BinaryFileDB store(String path, MultipartFile file, String fileName, String engineVersion, Boolean forceOverride, Boolean isElement) throws IOException, NoSuchAlgorithmException;
    BinaryFileDB getById(Long projectId);

    BinaryFileDB save(BinaryFileDB binaryFileDB);
    boolean storeCoverPage(String path, Long projectId, MultipartFile coverPageFile, String fileName, boolean isElement) throws IOException;

    void delete(String path) throws IOException;
}
