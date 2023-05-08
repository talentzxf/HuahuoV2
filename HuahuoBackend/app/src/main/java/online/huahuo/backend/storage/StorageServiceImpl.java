package online.huahuo.backend.storage;

import lombok.RequiredArgsConstructor;
import online.huahuo.backend.db.BinaryFileRepository;
import online.huahuo.backend.db.FileType;
import online.huahuo.backend.db.BinaryFileDB;
import online.huahuo.backend.exception.DuplicateFileException;
import online.huahuo.backend.utils.Utils;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.stereotype.Service;
import org.springframework.web.multipart.MultipartFile;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.security.NoSuchAlgorithmException;
import java.util.Date;

@Service
@RequiredArgsConstructor
public class StorageServiceImpl implements StorageService {

    final private String HUAHUO_POSTFIX = ".hua";
    final private BinaryFileRepository fileRepository;

    @Value("${huahuo.backend.datafilepath}")
    private String dataFilePath;

    String getPath() {
        if (dataFilePath.endsWith(File.separator))
            return dataFilePath;
        return dataFilePath + File.separator;
    }

    @Override
    public BinaryFileDB store(String path, MultipartFile file, Boolean forceOverride, Boolean isElement) throws IOException, NoSuchAlgorithmException {
        String fileName = file.getOriginalFilename();
        FileType fileType = isElement ? FileType.ELEMENT : FileType.PROJECT;
        String savePath = getPath() + path + File.separator + fileType + File.separator;
        String absoluteFilePath = savePath + fileName + HUAHUO_POSTFIX;


        if (!forceOverride) { // Don't override if the file exists and forceOverride = false.
            if (new File(absoluteFilePath).exists()) {
                throw new DuplicateFileException(fileName);
            }
        }

        new File(savePath).mkdirs();

        Files.write(Paths.get(absoluteFilePath), file.getBytes());

        // Get the userId from the JWT token.
        Authentication authentication = SecurityContextHolder.getContext().getAuthentication();
        String username = authentication.getName();

        String fileHash = Utils.hashBytes(file.getBytes());

        BinaryFileDB fileDB = fileRepository.findByCreatedByAndFileTypeAndName(username, fileType, fileName);

        // TODO: Read the version from the file.
        if (fileDB == null)
            fileDB = new BinaryFileDB(fileName, file.getContentType(), "0.0.1", username, absoluteFilePath, fileHash, "", fileType);
        else {
            fileDB.setChecksum(fileHash);
            fileDB.setModifiedTime(new Date());
        }

        return fileRepository.save(fileDB);
    }

    @Override
    public boolean storeCoverPage(String path, Long projectId, MultipartFile coverPageFile, boolean isElement) throws IOException {
        BinaryFileDB binaryFileDB = fileRepository.getReferenceById(projectId);

        String fileName = coverPageFile.getOriginalFilename();

        FileType fileType = isElement ? FileType.ELEMENT : FileType.PROJECT;
        String savePath = getPath() + path + File.separator + fileType + File.separator;
        String absoluteCoverPageFilePath = savePath + fileName;

        new File(savePath).mkdirs();
        Files.write(Paths.get(absoluteCoverPageFilePath), coverPageFile.getBytes());

        binaryFileDB.setCoverPagePath(absoluteCoverPageFilePath);
        fileRepository.save(binaryFileDB);
        return true;
    }

    @Override
    public BinaryFileDB getById(Long projectId) {
        return fileRepository.getReferenceById(projectId);
    }

    @Override
    public BinaryFileDB save(BinaryFileDB binaryFileDB) {
        return fileRepository.save(binaryFileDB);
    }
}
