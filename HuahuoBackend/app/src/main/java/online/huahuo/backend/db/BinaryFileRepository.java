package online.huahuo.backend.db;

import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.repository.PagingAndSortingRepository;
import org.springframework.stereotype.Repository;

import java.util.List;

@Repository
public interface BinaryFileRepository extends PagingAndSortingRepository<BinaryFileDB, Long>, JpaRepository<BinaryFileDB, Long> {
    List<BinaryFileDB> findByCreatedByAndFileTypeAndStatus(String createdBy, FileType fileType, ProjectStatus status,  Pageable pageable);
    BinaryFileDB findByCreatedByAndFileTypeAndName(String createdBy, FileType fileType, String projectName);
    int countByCreatedByAndFileTypeAndStatus(String createdBy, FileType fileType, ProjectStatus status);
}
