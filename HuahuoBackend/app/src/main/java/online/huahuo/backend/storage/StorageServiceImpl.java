package online.huahuo.backend.storage;

import lombok.AllArgsConstructor;
import lombok.RequiredArgsConstructor;
import online.huahuo.backend.db.FileRepository;
import online.huahuo.backend.db.ProjectFileDB;
import online.huahuo.backend.utils.Utils;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.stereotype.Service;
import org.springframework.web.multipart.MultipartFile;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.security.NoSuchAlgorithmException;

@Service
@RequiredArgsConstructor
public class StorageServiceImpl implements StorageService{

    final private FileRepository fileRepository;

    @Value("${huahuo.backend.datafilepath}")
    private String dataFilePath;

    String getPath() {
        if (dataFilePath.endsWith(File.separator))
            return dataFilePath;
        return dataFilePath + File.separator;
    }

    @Override
    public ProjectFileDB store(String path, MultipartFile file) throws IOException, NoSuchAlgorithmException {
        String fileName = file.getOriginalFilename();
        String savePath = getPath() + path + File.separator;
        String absoluteFilePath = savePath + fileName;

        if(new File(absoluteFilePath).exists()){
            throw new IOException("File already exists!");
        }

        new File(savePath).mkdirs();

        File targetFile = new File(absoluteFilePath);
        try (OutputStream os = new FileOutputStream(targetFile)) {
            os.write(file.getBytes());
        }

        // Get the userId from the JWT token.
        Authentication authentication = SecurityContextHolder.getContext().getAuthentication();
        String username = authentication.getName();

        String fileHash = Utils.hashBytes(file.getBytes());

        // TODO: Read the version from the file.
        ProjectFileDB fileDB = new ProjectFileDB(fileName, file.getContentType(), "0.0.1", username, absoluteFilePath, fileHash);

        return fileRepository.save(fileDB);
    }

    @Override
    public ProjectFileDB getById(Long projectId) {
        return fileRepository.getReferenceById(projectId);
    }

}
