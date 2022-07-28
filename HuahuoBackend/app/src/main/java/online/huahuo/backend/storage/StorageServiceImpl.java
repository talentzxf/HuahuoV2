package online.huahuo.backend.storage;

import lombok.AllArgsConstructor;
import online.huahuo.backend.db.FileDB;
import online.huahuo.backend.db.FileRepository;
import org.springframework.core.io.Resource;
import org.springframework.stereotype.Service;
import org.springframework.web.multipart.MultipartFile;

import java.io.IOException;
import java.nio.file.Path;
import java.util.stream.Stream;

@Service
@AllArgsConstructor
public class StorageServiceImpl implements StorageService{
    private FileRepository fileRepository;

    @Override
    public FileDB store(MultipartFile file) throws IOException {
        String fileName = file.getOriginalFilename();
        FileDB fileDB = new FileDB(fileName, file.getContentType(), file.getBytes());

        return fileRepository.save(fileDB);
    }

    @Override
    public FileDB getById(Long projectId) {
        return fileRepository.getReferenceById(projectId);
    }

}
