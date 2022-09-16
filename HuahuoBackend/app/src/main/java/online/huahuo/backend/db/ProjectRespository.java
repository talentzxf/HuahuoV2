package online.huahuo.backend.db;

import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.domain.Sort;
import org.springframework.data.repository.PagingAndSortingRepository;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

import java.util.List;

@Repository
public interface ProjectRespository extends PagingAndSortingRepository<ProjectFileDB, Long> {
    List<ProjectFileDB> findByCreatedByAndStatus(String createdBy, ProjectStatus status, Pageable pageable);
    ProjectFileDB findByCreatedByAndName(String createdBy, String projectName);
    int countByCreatedByAndStatus(String createdBy, ProjectStatus status);
}
