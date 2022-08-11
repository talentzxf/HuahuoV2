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
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.context.SecurityContextHolder;
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
    public FileUploadStatus handleFileUpload(@RequestParam("file")MultipartFile file, @RequestParam(value = "force_override", required = false) Boolean forceOverride) throws IOException, NoSuchAlgorithmException {
        Authentication authentication = SecurityContextHolder.getContext().getAuthentication();
        String username = authentication.getName();

        if(forceOverride == null){ // By default, override previous project
            forceOverride = true;
        }

        ProjectFileDB fileDB = storageService.store(username, file, forceOverride);
        return new FileUploadStatus(fileDB.getId(), true, "File uploaded successfully!");
    }
}
