package online.huahuo.backend.storage;

import lombok.AllArgsConstructor;
import lombok.RequiredArgsConstructor;
import online.huahuo.backend.db.ProjectFileDB;
import online.huahuo.backend.db.FileRepository;
import online.huahuo.backend.utils.Utils;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;
import org.springframework.web.multipart.MultipartFile;

import java.io.File;
import java.io.IOException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

@Service
@RequiredArgsConstructor
public class StorageServiceImpl implements StorageService {
    private FileRepository fileRepository;

    @Value("{huahuo.backend.datafilepath}")
    private String dataFilePath;

    String getPath() {
        if (dataFilePath.endsWith(File.separator))
            return dataFilePath;
        return dataFilePath + File.separator;
    }

    @Override
    public ProjectFileDB store(String path, MultipartFile file) throws IOException, NoSuchAlgorithmException {
        String fileName = file.getOriginalFilename();
        String savePath = getPath() + path + File.separator + fileName;

        // Check if the file already exists.
        ProjectFileDB fileDB = new ProjectFileDB(fileName, file.getContentType(), "0.0.1", 0L);
//
//        return fileRepository.save(fileDB);
        return null;
    }

    @Override
    public ProjectFileDB getById(Long projectId) {
        return fileRepository.getReferenceById(projectId);
    }

}
