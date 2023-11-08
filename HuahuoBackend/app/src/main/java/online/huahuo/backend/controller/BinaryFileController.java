package online.huahuo.backend.controller;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.extern.slf4j.Slf4j;
import online.huahuo.backend.db.*;
import online.huahuo.backend.storage.StorageService;
import org.springframework.core.io.ByteArrayResource;
import org.springframework.core.io.Resource;
import org.springframework.data.domain.PageRequest;
import org.springframework.data.domain.Pageable;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.security.access.prepost.PreAuthorize;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.*;
import org.springframework.web.multipart.MultipartFile;
import com.google.common.io.Resources;

import java.io.IOException;
import java.net.URL;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.security.NoSuchAlgorithmException;
import java.util.List;

@Data
@AllArgsConstructor
class BinaryFileCallStatus {
    private long fileId;
    private boolean succeeded;
    private String msg;
}


@Data
class BinaryFileExistResponse {
    private String fileName;
    private Boolean exist;
}

@Slf4j
@Controller
@AllArgsConstructor
public class BinaryFileController {
    private final StorageService storageService;
    @javax.annotation.Resource
    private BinaryFileRepository binaryFileRepository;

    @javax.annotation.Resource
    private ElementRepository elementRepository;


    @PreAuthorize("hasRole('CREATOR')")
    @ResponseBody
    @PostMapping(value = "/binaryfiles/upload",
            consumes = {MediaType.MULTIPART_FORM_DATA_VALUE})
    public BinaryFileCallStatus uploadFile(@RequestParam MultipartFile file,
                                           @RequestParam String fileName,
                                           @RequestParam String engineVersion,
                                           @RequestParam(required = false, defaultValue = "true") Boolean forceOverride,
                                           @RequestParam(required = false, defaultValue = "false") Boolean isElement
    ) throws IOException, NoSuchAlgorithmException {
        Authentication authentication = SecurityContextHolder.getContext().getAuthentication();
        String username = authentication.getName();

        BinaryFileDB fileDB = storageService.store(username, file, fileName, engineVersion, forceOverride, isElement);
        return new BinaryFileCallStatus(fileDB.getId(), true, "File uploaded successfully!");
    }

    @ResponseBody
    @PutMapping(value = "/binaryfiles/{fileId}/description",
            consumes = "text/plain")
    public BinaryFileCallStatus updateBinaryFileDescription(@PathVariable Long fileId, @RequestBody String description) {
        BinaryFileDB fileDB = storageService.getById(fileId);
        fileDB.setDescription(description);
        storageService.save(fileDB);

        return new BinaryFileCallStatus(fileDB.getId(), true, "Description changed successfully");
    }


    @ResponseBody
    @GetMapping(value = "/binaryfiles/{fileId}",
            produces = "application/octet-stream")
    public ResponseEntity<Resource> downloadBinaryFile(@PathVariable Long fileId) throws IOException {
        BinaryFileDB fileDB = storageService.getById(fileId);

        String fullPath = fileDB.getFullPath();
        byte[] fileData = Files.readAllBytes(Paths.get(fullPath));

        ByteArrayResource resource = new ByteArrayResource(fileData);

        HttpHeaders headers = new HttpHeaders();
        headers.add(HttpHeaders.CONTENT_DISPOSITION, "attachment;filename=\"" + fileDB.getName() + "\"");
        headers.add("X-Suggested-Filename", fileDB.getName());

        return ResponseEntity.ok()
                .headers(headers)
                .contentLength(fileData.length)
                .contentType(MediaType.APPLICATION_OCTET_STREAM)
                .body(resource);
    }


    @ResponseBody
    @PreAuthorize("hasRole('READER')")
    @GetMapping("/binaryfiles")
    public ResponseEntity listBinaryFiles(@RequestParam(defaultValue = "0") int pageNumber,
                                          @RequestParam(defaultValue = "10") int pageSize,
                                          @RequestParam(defaultValue = "false") Boolean isElement) {
        try {


            Authentication authentication = SecurityContextHolder.getContext().getAuthentication();
            String username = authentication.getName();

            Pageable pageable = PageRequest.of(pageNumber, pageSize);

            FileType fileType = isElement ? FileType.ELEMENT : FileType.PROJECT;
            List<BinaryFileDB> resultList = null;

            if (fileType == FileType.PROJECT)
                resultList = binaryFileRepository.findByCreatedByAndFileTypeAndStatusOrderByModifiedTimeDesc(username, fileType, BinaryFileStatus.ACTIVE, pageable);
            else
                resultList = binaryFileRepository.findByFileTypeAndStatusOrderByModifiedTimeDesc(fileType, BinaryFileStatus.ACTIVE, pageable);

            int totalFileCount = binaryFileRepository.countByCreatedByAndFileTypeAndStatus(username, fileType, BinaryFileStatus.ACTIVE);
            ListBinaryFileResult listBinaryFileResult = new ListBinaryFileResult();
            listBinaryFileResult.setBinaryFiles(resultList);
            listBinaryFileResult.setTotalCount(totalFileCount);
            return ResponseEntity.ok()
                    .body(listBinaryFileResult);
        } catch (IllegalArgumentException e) {
            return ResponseEntity.badRequest().body(e.getMessage());
        }
    }

    @ResponseBody
    @PreAuthorize("hasRole('READER')")
    @GetMapping("/binaryfiles/existFile")
    ResponseEntity<BinaryFileExistResponse> existFile(@RequestParam String fileName) {
        Authentication authentication = SecurityContextHolder.getContext().getAuthentication();
        String username = authentication.getName();

        BinaryFileDB binaryFileDB = binaryFileRepository.findByCreatedByAndFileTypeAndName(username, FileType.PROJECT, fileName);

        BinaryFileExistResponse response = new BinaryFileExistResponse();
        response.setFileName(fileName);

        if (binaryFileDB != null) {
            response.setExist(true);
        } else {
            response.setExist(false);
        }
        return new ResponseEntity<>(response, HttpStatus.OK);
    }

    @ResponseBody
    @PostMapping(value = "/binaryfiles/{fileId}/coverPage",
            consumes = {MediaType.MULTIPART_FORM_DATA_VALUE})
    public BinaryFileCallStatus uploadCoverPage(@RequestParam MultipartFile coverPageFile,
                                                @RequestParam String fileName,
                                                @PathVariable Long fileId,
                                                @RequestParam(defaultValue = "false", required = false) Boolean isElement) throws IOException {
        Authentication authentication = SecurityContextHolder.getContext().getAuthentication();
        String username = authentication.getName();
        storageService.storeCoverPage(username, fileId, coverPageFile, fileName, isElement);
        return new BinaryFileCallStatus(fileId, true, "Binary file cover page uploaded successfully!");
    }

    @ResponseBody
    @GetMapping("/binaryfiles/coverPageNotFound")
    public ResponseEntity<Resource> coverPageNotFound() throws IOException {
        URL url = Resources.getResource("image-not-found-icon.png");
        byte[] fileData = Resources.toByteArray(url);
        ByteArrayResource resource = new ByteArrayResource(fileData);

        HttpHeaders headers = new HttpHeaders();
        headers.add(HttpHeaders.CONTENT_DISPOSITION, "attachment;filename=image-not-found-icon.png");

        return ResponseEntity.ok()
                .headers(headers)
                .contentLength(fileData.length)
                .contentType(MediaType.APPLICATION_OCTET_STREAM)
                .body(resource);
    }

    @ResponseBody
    @PreAuthorize("hasRole('CREATOR')")
    @DeleteMapping("/binaryfiles/{fileId}")
    @ResponseStatus(HttpStatus.OK)
    public ResponseEntity deleteBinaryFile(@PathVariable Long fileId) throws IOException {
        Authentication authentication = SecurityContextHolder.getContext().getAuthentication();
        String username = authentication.getName();

        BinaryFileDB binaryFileDB = binaryFileRepository.findById(fileId).get();
        if (!binaryFileDB.getCreatedBy().equals(username)) {
            return ResponseEntity.badRequest().body("Binary file creator mismatch!");
        }

        String fileDataPath = binaryFileDB.getFullPath();
        String fileDoverPathPath = binaryFileDB.getCoverPagePath();

        try {
            Files.delete(Path.of(fileDataPath));
            Files.delete(Path.of(fileDoverPathPath));
        } finally {
            if (binaryFileDB.getFileType().equals(FileType.ELEMENT)) {
                ElementDB elementDB = elementRepository.getByBinaryFileDB(binaryFileDB);
                if (elementDB != null) {
                    String elementId = elementDB.getElementId();
                    elementRepository.deleteById(elementId);
                }

            }
            binaryFileRepository.deleteById(fileId);
        }

        return ResponseEntity.ok("Binary file successfully deleted!");
    }

    public ResponseEntity JumpToNotFoundImage() {
        HttpHeaders headers = new HttpHeaders();
        headers.add("Location", "/binaryfiles/coverPageNotFound");

        // 302 jump to not found image
        return new ResponseEntity<byte[]>(null, headers, HttpStatus.FOUND);
    }

    // TODO: Add Auth!!!
    // This is not REST call, don't marshal response.
    // @PreAuthorize("hasRole('READER')")
    @GetMapping("/binaryfiles/{fileId}/coverPage")
    public ResponseEntity getBinaryFileCoverPage(@PathVariable Long fileId) throws IOException {
        try {
            BinaryFileDB fileDB = storageService.getById(fileId);
            String fullPath = fileDB.getCoverPagePath();
            if (fullPath == null)
                return JumpToNotFoundImage();

            byte[] fileData = Files.readAllBytes(Paths.get(fullPath));

            ByteArrayResource resource = new ByteArrayResource(fileData);

            HttpHeaders headers = new HttpHeaders();
            headers.add(HttpHeaders.CONTENT_DISPOSITION, "attachment;filename=\"" + fileDB.getName() + "\"");
            headers.add("X-Suggested-Filename", fileDB.getName());

            return ResponseEntity.ok()
                    .headers(headers)
                    .contentLength(fileData.length)
                    .contentType(MediaType.APPLICATION_OCTET_STREAM)
                    .body(resource);
        } catch (Exception e) {
            log.error("Exception happened, redirect to not found page.", e);
            return JumpToNotFoundImage();
        }
    }

}
