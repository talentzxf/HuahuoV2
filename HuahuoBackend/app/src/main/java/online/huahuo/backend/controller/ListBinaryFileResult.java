package online.huahuo.backend.controller;

import lombok.Data;
import online.huahuo.backend.db.BinaryFileDB;

import java.util.List;

@Data
public class ListBinaryFileResult {
    private int totalCount;
    private List<BinaryFileDB> binaryFiles;
}