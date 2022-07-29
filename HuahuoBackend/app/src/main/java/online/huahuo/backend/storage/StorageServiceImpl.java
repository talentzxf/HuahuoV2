package online.huahuo.backend.storage;

import lombok.AllArgsConstructor;
import online.huahuo.backend.db.ProjectFileDB;
import online.huahuo.backend.db.FileRepository;
import org.springframework.stereotype.Service;
import org.springframework.web.multipart.MultipartFile;

import java.io.IOException;

@Service
@AllArgsConstructor
public class StorageServiceImpl implements StorageService{
    private FileRepository fileRepository;

    @Override
    public ProjectFileDB store(MultipartFile file) throws IOException {
        String fileName = file.getOriginalFilename();

        ProjectFileDB fileDB = new ProjectFileDB(fileName, "0.0.1", file.getContentType(), file.getBytes(), 0L);

        return fileRepository.save(fileDB);
    }

    @Override
    public ProjectFileDB getById(Long projectId) {
        return fileRepository.getReferenceById(projectId);
    }

}
