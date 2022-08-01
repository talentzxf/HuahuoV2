package online.huahuo.backend.controller;

import lombok.AllArgsConstructor;
import lombok.Data;
import online.huahuo.backend.db.ProjectFileDB;
import online.huahuo.backend.storage.StorageService;
import org.springframework.core.io.ByteArrayResource;
import org.springframework.core.io.Resource;
import org.springframework.http.HttpHeaders;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.*;
import org.springframework.web.multipart.MultipartFile;

import java.io.IOException;
import java.security.NoSuchAlgorithmException;

@Data
@AllArgsConstructor
class FileUploadStatus{
    private long fileId;
    private boolean succeeded;
    private String msg;
}

@Controller
@AllArgsConstructor
public class FileUploadController {
    private final StorageService storageService;

    @ResponseBody
    @PostMapping("/projects/upload")
    public FileUploadStatus handleFileUpload(@RequestParam("file")MultipartFile file) throws IOException, NoSuchAlgorithmException {
        ProjectFileDB fileDB = storageService.store(file);
        return new FileUploadStatus(fileDB.getId(), true, "File uploaded successfully!");
    }

//    @GetMapping("/projects/{projectId}")
//    public ResponseEntity<Resource>  downloadProject(@PathVariable Long projectId){
//        ProjectFileDB fileDB = storageService.getById(projectId);
//        byte[] fileData = fileDB.getData();
//        ByteArrayResource resource = new ByteArrayResource(fileData);
//
//        HttpHeaders headers = new HttpHeaders();
//        headers.add(HttpHeaders.CONTENT_DISPOSITION, "attachment;filename=\"" + fileDB.getName() + "\"");

//        return ResponseEntity.ok()
//                .headers(headers)
//                .contentLength(fileData.length)
//                .contentType(MediaType.APPLICATION_OCTET_STREAM)
//                .body(resource);
//    }
}
