package online.huahuo.backend.db;

import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.domain.Sort;
import org.springframework.data.repository.PagingAndSortingRepository;
import org.springframework.stereotype.Repository;

import java.util.List;

@Repository
public interface ProjectRespository extends PagingAndSortingRepository<ProjectFileDB, Long> {
    List<ProjectFileDB> findAllByCreatedBy(String userName, Pageable pageable);
}
