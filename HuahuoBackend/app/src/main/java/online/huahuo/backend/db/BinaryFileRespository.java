package online.huahuo.backend.db;

import org.springframework.data.domain.Pageable;
import org.springframework.data.repository.PagingAndSortingRepository;
import org.springframework.stereotype.Repository;

import java.util.List;

@Repository
public interface BinaryFileRespository extends PagingAndSortingRepository<BinaryFileDB, Long> {
    List<BinaryFileDB> findByCreatedByAndStatus(String createdBy, ProjectStatus status, Pageable pageable);
    BinaryFileDB findByCreatedByAndName(String createdBy, String projectName);
    int countByCreatedByAndStatus(String createdBy, ProjectStatus status);
}
