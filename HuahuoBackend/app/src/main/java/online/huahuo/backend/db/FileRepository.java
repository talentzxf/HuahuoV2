package online.huahuo.backend.db;

import org.springframework.data.jpa.repository.JpaRepository;

public interface FileRepository extends JpaRepository<ProjectFileDB, Long> {
    ProjectFileDB findByFullPath(String fullPath);
}
